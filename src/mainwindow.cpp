#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/database.h"
#include "headers/highlighter.h"
#include "headers/encryption.h"
#include "mdLite/token.h"
#include "mdLite/tokenizer.h"
#include <QClipboard>
#include <QCheckBox>
#include <QDir>
#include <QDialogButtonBox>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QRadioButton>
#include <QShortcut>
#include <QTextDocumentFragment>
#include <sstream>
#include <fstream>

#ifdef Q_OS_DARWIN
    #include "headers/macosuihandler.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
                QMainWindow(parent),
                ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Tab), this);
    QObject::connect(shortcut, &QShortcut::activated, this, &MainWindow::reverseTab);

    ui->noteText->installEventFilter(this);

    init();
}

MainWindow::~MainWindow() {
    delete ui;
}

// Overriding `undo (control/command + Z)` event to prevent
// list releted methods from running on empty list item lines
// when undoing changes
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->key() == 'Z') {
            ui->noteText->blockSignals(true);
            if (keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                ui->noteText->redo();
            } else {
                ui->noteText->undo();
            }
            ui->noteText->blockSignals(false);
            event->ignore();
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::init() {    
    darkStyles = "QPlainTextEdit { padding: 5% 5% 0 5%; color: white; background-color: #252525; border:none; }";
    lightStyles = "QPlainTextEdit { padding: 5% 5% 0 5%; color: #454545; background-color: #FAFAFA; border:none; }";

    this->setStyleSheet("QMainWindow { background-color: #505050; border: none; }");
    ui->splitter->setStretchFactor (0,0);
    ui->splitter->setStretchFactor (1,1);
    ui->splitter->setSizes(QList<int>() << 200 << 160000);
    ui->noteText->setStyleSheet(darkStyles);
    ui->openedNotePath->setStyleSheet("color: white;");
    ui->actionDark->setChecked(true);
    ui->fileList->setWordWrap(true);
    ui->noteText->setAcceptDrops(false);
    setAcceptDrops(true);

#ifdef Q_OS_DARWIN
    ui->fileList->setAttribute(Qt::WA_MacShowFocusRect, false);
#endif

    // changing font size is required to fix the inconsistent
    // word wrapping/ellipsis of fileList items
    QFont font = ui->fileList->font();
    font.setPointSize(13);
    ui->fileList->setFont(font);

    highlighter = new Highlighter(this);
    highlighter->setDocument(ui->noteText->document());

    fileSaved = true;
    openedFile = true;
    showWordCount = false;
    changeCount = 0;

    if (db.fontConfigExists()) {
        QFont font;
        font.fromString(db.getFontConfig());
        ui->noteText->setFont(font);
    }

    if (db.displayModeExists() && db.getDisplayMode() == "light") {
        setDisplayModeLight();
    }

    this->paths = db.getRecents();
    resetFileList();
}

void MainWindow::openFile(QString &filePath) {
    if (filePath != "") {
        if (!fileSaved) {
            if (!fileSavePromt())
                return;
        }

        bool fileExists = QFileInfo::exists(filePath) && QFileInfo(filePath).isFile();

        if (!fileExists) {
            QMessageBox msgBox;
            msgBox.setText("Can't find the note");
            msgBox.setInformativeText("It seems like the note '.md' file was deleted!\n Do you want to delete the record?");
            msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
            msgBox.setDefaultButton(QMessageBox::Yes);

            // this is required to make the MessageBox wider
            QSpacerItem* horizontalSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
            QGridLayout* layout = (QGridLayout*)msgBox.layout();
            layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

            if (msgBox.exec() == QMessageBox::Yes)
                db.deleteTaggedPath(filePath);

            return;
        }

        fileName = filePath;
        ui->openedNotePath->setText("Path: " + filePath);
        std::ifstream file(filePath.toStdString());
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();
        fileSaved = true;
        openedFile = true;
        changeCount = 0;
        ui->noteText->setPlainText(QString::fromStdString(content));

        if (!db.noteExists(filePath)) {
            Tokenizer tokenizer;
            tokenizer.tokenize(content);
            tagArr.clear();
            tagArr = tokenizer.getTags();

            if (tagArr.size() > 0) {
                db.insertNoteWithTags(filePath, tagArr);
            } else {
                db.insertNote(filePath);
            }
        }

        if (!db.recentNoteExists(filePath)) {
            db.insertRecentNote(db.getRowId(filePath.toStdString()));
        } else {
            db.updateOpenedDate(filePath);
        }

#ifdef Q_OS_DARWIN
        setWindowModified(false);
#endif
    } else {
#ifdef Q_OS_DARWIN
        setWindowModified(true);
#endif
    }
}

bool MainWindow::fileSavePromt() {
    QMessageBox msgBox;
    msgBox.setText("Unsaved Note");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Save) {
        if (!saveFile())
            return false;
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

bool MainWindow::saveFile() {
    try {
        if (!openedFile || fileName.isEmpty() || fileName == "") {
            QString path = QDir::homePath();
            if (db.lastOpenPathExists())
                path = db.getLastOpenPath();
            fileName = QFileDialog::getSaveFileName(this, tr("Save Note"), path, tr("Markdown (*.md)"));

            if (fileName.isEmpty())
                return false;

            QFileInfo info(fileName);
            db.insertLastOpenPath(info.absolutePath());
        }
    } catch (std::exception& ex) {
        return false;
    }

    try {
        std::ofstream out(fileName.toUtf8().constData());
        std::string output = ui->noteText->toPlainText().toUtf8().constData();
        out << output;
        out.close();
        fileSaved = true;
        openedFile = true;

#ifdef Q_OS_DARWIN
        setWindowModified(false);
#endif

        Tokenizer tokenizer;
        tokenizer.tokenize(output);
        tagArr = tokenizer.getTags();        

        if (!db.noteExists(fileName)) {
            if (tagArr.size() > 0) {
                db.insertNoteWithTags(fileName, tagArr);
            } else {
                db.insertNote(fileName);
            }
        } else {
            db.deleteTags(db.getRowId(fileName.toUtf8().constData()));
            if (tagArr.size() > 0)
                db.updateTags(fileName, tagArr);            
        }

        resetFileList();
    } catch (std::exception& ex) {
        fileSaved = false;
        displayMessage("There was an error! please try again.");
        return false;
    }

    return true;
}

void MainWindow::resetFileList() {
    ui->fileListOptions->blockSignals(true);
    ui->fileList->clear();
    ui->fileListOptions->clear();
    ui->fileListOptions->addItem("Recent Notes");
    ui->fileListOptions->addItem("All Notes");

    QVector<QString> tags = db.getTags();

    setFileList();

    if (tags.size() > 0) {
        QStringList tagList = tags.toList();
        tagList.removeDuplicates();
        ui->fileListOptions->addItems(tagList);
    }

    ui->fileListOptions->blockSignals(false);
}

void MainWindow::setFileList() {
#ifdef Q_OS_WIN
    QString splitPattern = "\\";
#else
    QString splitPattern = "\/";
#endif

    if (!this->paths.empty()) {
        ui->fileList->clear();
        for (auto& path : this->paths) {
            QStringList list = path.split(splitPattern);
            if (!list.empty()) {
                QRegularExpression regex("(.+?)(\\.[^.]*$|$)");
                QRegularExpressionMatch match = regex.match(list.at(list.size() - 1));
                ui->fileList->addItem(match.captured(1));
            }
        }
    }
}

void MainWindow::openedFileHelper() {
    QString path = QDir::homePath();
    if (db.lastOpenPathExists())
        path = db.getLastOpenPath();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("Markdown (*.md)"));
    QFileInfo info(fileName);
    if (fileName.size() > 0 && info.isFile()) {
        openFile(fileName);
        this->paths = db.getRecents();
        resetFileList();
        db.insertLastOpenPath(info.absolutePath());
    }
}

void MainWindow::setDisplayModeLight() {
    ui->actionLight->setChecked(true);
    ui->actionDark->setChecked(false);
    ui->noteText->setStyleSheet(lightStyles);
    db.insertDisplayMode("light");
}

void MainWindow::setDisplayModeDark() {
    ui->actionDark->setChecked(true);
    ui->actionLight->setChecked(false);
    ui->noteText->setStyleSheet(darkStyles);
    db.insertDisplayMode("dark");
}

QString MainWindow::getWordCount() {
    int wordCount = ui->noteText->toPlainText().split(QRegExp("(\\s|\\n|\\r)+"), QString::SkipEmptyParts).count();
    return QString::number(wordCount);
}

std::string MainWindow::getFileContent(std::string path) {
    try {
        std::ifstream ifs(path);
        std::string output((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        ifs.close();
        return output;
    } catch(const std::exception& e) { }

    return "";
}

std::vector<std::string> MainWindow::split(std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter))
       tokens.push_back(token);

    return tokens;
}

void MainWindow::makeList(std::string type) {
    std::string selection = ui->noteText->textCursor().selection().toPlainText().toStdString();
    std::vector<std::string> lines = split(selection, '\n');

    if (type == "unordered") {
        for (auto &line : lines) {
            if (!line.empty())
                line = "* " + line;
        }
    } else if (type == "ordered") {
        for (int i = 0; i < lines.size(); i++)
            lines.at(i) = std::to_string(i + 1) + ". " + lines.at(i);
    }

    std::string text = "";

    for (auto &line : lines)
        text += line + "\n";

    ui->noteText->insertPlainText(QString::fromStdString(text));
}

bool MainWindow::checkListItem(QString &line) {
    QRegularExpression regex1("^\\s*\\*\\s");
    QRegularExpression regex2("^\\s*\\d*\\.\\s");
    return (regex1.match(line).hasMatch() || regex2.match(line).hasMatch());
}

bool MainWindow::checkUnorderedListItem(QString &line) {
    QRegularExpression regex("^\\s*\\*\\s");
    return (regex.match(line).hasMatch());
}

int MainWindow::getSpaceCount(QString &line) {
    int count = 0;
    for (auto& c : line) {
        if (c == ' ') {
            count += 1;
        } else {
            break;
        }
    }
    return count;
}

QString MainWindow::getNextNumber(QString &line) {
    QString number = "";
    QString trimmedLine = line.trimmed();
    for (auto& c : trimmedLine) {
        if (!c.isDigit())
            break;
        number += c;
    }
    if (number != "") {
        try {
            int newNumber = number.toInt() + 1;
            return QString::number(newNumber);
        } catch (std::exception &ex) {
            return "";
        }
    } else {
        return "";
    }
}

void MainWindow::reverseTab() {
    QTextCursor tempCursor = ui->noteText->textCursor();
    QString line = tempCursor.block().text();
    if (checkListItem(line) && getSpaceCount(line) >= 4) {
        int pos = tempCursor.positionInBlock();
        ui->noteText->blockSignals(true);
        tempCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        tempCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);
        tempCursor.removeSelectedText();
        tempCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos - 4);
        ui->noteText->setTextCursor(tempCursor);
        ui->noteText->blockSignals(false);
    }
}

void MainWindow::displayMessage(QString message) {
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

void MainWindow::on_newNoteBtn_clicked() {
    if (openedFile && !fileSaved) {
        if (!fileSavePromt())
            return;
    }
    fileSaved = false;
    openedFile = false;
    fileName = "";
    ui->openedNotePath->setText("");
    ui->noteText->setPlainText("# New note");
    changeCount = 0;
}

void MainWindow::on_openNoteBtn_clicked() {
    openedFileHelper();
}

void MainWindow::on_actionOpen_triggered() {
    openedFileHelper();
}

void MainWindow::on_actionSave_triggered() {
    if (!fileSaved)
        saveFile();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString path = urls.first().toLocalFile();
    if (path.isEmpty())
        return;

    QFileInfo info(path);

    if (info.isFile() && info.suffix() == "md") {
        openFile(path);
        this->paths = db.getRecents();
        resetFileList();
    }

}

void MainWindow::on_actionInsert_Table_triggered() {
    int rows = 0;
    int cols = 0;

    QDialog dialog(this);
    dialog.setWindowTitle("Insert table");

    QFormLayout form(&dialog);
    QList<QLineEdit *> fields;

    QLineEdit *lineEdit = new QLineEdit(&dialog);
    form.addRow("Rows", lineEdit);
    fields << lineEdit;

    lineEdit = new QLineEdit(&dialog);
    form.addRow("Colums", lineEdit);
    fields << lineEdit;

    QGroupBox *groupBox = new QGroupBox();
    QRadioButton *radio1 = new QRadioButton(tr("Left"));
    QRadioButton *radio2 = new QRadioButton(tr("Center"));
    QRadioButton *radio3 = new QRadioButton(tr("Right"));

    radio1->setChecked(true);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(radio1);
    hbox->addWidget(radio2);
    hbox->addWidget(radio3);
    hbox->addStretch(0);
    groupBox->setLayout(hbox);

    form.addRow("Text align", groupBox);

    QCheckBox *checkBox = new QCheckBox(&dialog);
    form.addRow("Minified", checkBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        std::string r = fields.at(0)->text().toStdString();
        std::string c = fields.at(1)->text().toStdString();

        if (!r.empty() && !c.empty()) {
            try {
                rows = std::stoi(r);
                cols = std::stoi(c);
            } catch(std::exception &ex) {
                // ignore invalid input
                return;
            }

            bool minified = checkBox->isChecked();

            std::string align = " --- ";

            if (radio2->isChecked())
                align = ":---:";

            if (radio3->isChecked())
                align = " ---:";

            if (rows > 100)
                rows = 100;

            if (cols > 25)
                cols = 25;

            std::string table;

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    if (i == 1) {
                        table += "|" + align;
                    } else {
                        table += "|";
                        if (!minified)
                            table += "     ";
                    }

                    if (j == cols - 1)
                        table += "|";
                }

                table += "\n";
            }

            ui->noteText->insertPlainText(QString::fromStdString(table));
        }
    }
}

void MainWindow::on_actionMake_Unordered_List_triggered() {
    makeList("unordered");
}

void MainWindow::on_actionMake_Ordered_List_triggered() {
    makeList("ordered");
}

void MainWindow::on_actionCopy_selection_as_HTML_triggered() {
    QString selection = ui->noteText->textCursor().selection().toPlainText() + "\n";
    std::string text = selection.toStdString();
    Tokenizer tokenizer;
    tokenizer.tokenize(text);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::fromStdString(tokenizer.getHTML()));
}

void MainWindow::on_actionExport_HTML_triggered() {
    std::string text = ui->noteText->toPlainText().toStdString() + "\n";
    Tokenizer tokenizer;
    tokenizer.tokenize(text);

    QString path = QDir::homePath();
    if (db.lastOpenPathExists())
        path = db.getLastOpenPath();

    QString htmlSavePath = QFileDialog::getSaveFileName(this, tr("Export HTML File"), path, tr("HTML (*.html)"));

    QFileInfo info(htmlSavePath);
    if (htmlSavePath.size() > 0 && info.isFile())
        db.insertLastOpenPath(info.absolutePath());

    std::ofstream out(htmlSavePath.toUtf8().constData());
    std::string headerPath = "html/header.html";
    std::string footerPath = "html/footer.html";

#ifdef Q_OS_DARWIN
    headerPath = QString(QApplication::applicationDirPath() + "/../Resources/header.html").toStdString();
    footerPath = QString(QApplication::applicationDirPath() + "/../Resources/footer.html").toStdString();
#endif

    std::string output = getFileContent(headerPath);
    output += tokenizer.getHTML();
    output += getFileContent(footerPath);
    out << output;
    out.close();
}

void MainWindow::on_actionEncrypt_note_triggered() {
    QString pswd = QInputDialog::getText(
                        0,
                        "Encrypt Note",
                        "Enter the passphrase...\nNOTE: Please remember your password! This can't be reversed without it.",
                        QLineEdit::Password
                    );
    QString pswd1 = QInputDialog::getText(
                        0,
                        "Encrypt Note",
                        "Enter the passphrase again...\nNOTE: Please remember your password! This can't be reversed without it.",
                        QLineEdit::Password
                    );

    if (pswd != pswd1) {
        displayMessage("Passphrase don't match!");
        return;
    }

    std::string input = ui->noteText->toPlainText().toStdString();
    std::string passphrase = pswd.toStdString();

    if (passphrase.empty())
        return;

    std::string encryptedText = Encryption::encrypt(input, passphrase);

    if (!encryptedText.empty()) {
        ui->noteText->setPlainText(QString::fromStdString(encryptedText));
    } else {
        displayMessage("Something went wrong during encryption...");
    }
}

void MainWindow::on_actionDecrypt_Note_triggered() {
    QString pswd = QInputDialog::getText(0, "Decrypt Note", "Enter the passphrase...", QLineEdit::Password);
    std::string input = ui->noteText->toPlainText().toStdString();
    std::string passphrase = pswd.toStdString();

    if (passphrase.empty())
        return;

    std::string decryptedText = Encryption::decrypt(input, passphrase);

    if (!decryptedText.empty()) {
        ui->noteText->setPlainText(QString::fromStdString(decryptedText));
    } else {
        displayMessage("It seems like your note is not encrypted...");
    }
}

void MainWindow::on_fileListOptions_currentTextChanged(const QString &arg1) {
    std::string tag = arg1.toUtf8().constData();
    if (tag == "Recent Notes" || tag == "All Notes") {
        ui->fileList->clear();
        if (tag == "All Notes") {
            this->paths = db.getNotes();
        } else {
            this->paths = db.getRecents();
        }
    } else {
        this->paths = db.getNotesByTag(tag);
    }

    setFileList();
}

void MainWindow::on_fileList_itemClicked(QListWidgetItem *item) {
    QString filePath = this->paths.at(ui->fileList->currentRow());
    if (fileSaved && filePath == fileName)
        return;
    openFile(filePath);
    if (ui->fileListOptions->currentText() == "Recent Notes") {
        this->paths = db.getRecents();
        resetFileList();
        ui->fileList->setCurrentIndex(ui->fileList->model()->index(0, 0));
    }
}

void MainWindow::on_noteText_textChanged() {
    changeCount += 1;
    fileSaved = (openedFile) ? (changeCount == 1) ? true : false : false;
#ifdef Q_OS_DARWIN
    if (!fileSaved)
        setWindowModified(true);
#endif

    if (showWordCount) {
        QString wordCountText = "Word Count: " + getWordCount();
        if (openedFile)
            wordCountText += " | Path: " + fileName;
        ui->openedNotePath->setText(wordCountText);
    }

    QTextBlock textBlock = ui->noteText->textCursor().block();    

    if (textBlock.text().length() == 0) {
        QString prevLine = textBlock.previous().text();
        if (checkListItem(prevLine)) {
            int spaceCount = getSpaceCount(prevLine);
            QString newLine = QString("").leftJustified(spaceCount, ' ');

            if (checkUnorderedListItem(prevLine)) {
                newLine += "* ";
            } else {
                newLine += getNextNumber(prevLine) + ". ";
            }

            ui->noteText->textCursor().insertText(newLine);
        }
    }
}

void MainWindow::on_actionChange_Font_triggered() {
    bool changed;
    QFont font = QFontDialog::getFont(&changed, ui->noteText->font());
    if (changed) {
        db.insertFontConfig(font.toString());
        ui->noteText->setFont(font);
    }
}

void MainWindow::on_actionLight_triggered() {
    setDisplayModeLight();
}

void MainWindow::on_actionDark_triggered() {
    setDisplayModeDark();
}

void MainWindow::closeEvent (QCloseEvent *event) {
    if (!fileSaved) {
        if (!fileSavePromt())
            event->ignore();
    }
}

void MainWindow::on_actionShow_Word_Count_triggered() {
    this->showWordCount = (!this->showWordCount);
    QString text = "";
    if (this->showWordCount) {
        text = "Word Count: " + getWordCount();
    }
    if (openedFile && !fileName.isEmpty()) {
        if (this->showWordCount)
            text += " | ";
        text += "Path: " + fileName;
    }
    ui->openedNotePath->setText(text);
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox msgBox;
    msgBox.setText("QuickNote v3.4.1");
    msgBox.setInformativeText("QuickNote is a simple note app with markdown support.\nhttps:\\\\www.github.com/dhamith93/QuickNote");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    QSpacerItem* horizontalSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    msgBox.exec();
}
