#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/database.h"
#include "headers/highlighter.h"
#include "headers/encryption.h"
#include "headers/config.h"
#include "src/libs/include/helpers.h"
#include "mdLite/token.h"
#include "mdLite/tokenizer.h"
#include "headers/displayconfigdialog.h"
#include <QClipboard>
#include <QCheckBox>
#include <QDir>
#include <QDialogButtonBox>
#include <QDebug>
#include <QFileDialog>
#include <QFontDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QRadioButton>
#include <QShortcut>
#include <QTextDocumentFragment>
#include <QTimer>

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

    Config::getInstance().init();
    QString noteDbPath = Config::getInstance().get(Config::getInstance().NOTE_DB_PATH);
    if (noteDbPath.isEmpty()) {
        QTimer::singleShot(1, this, SLOT(getDatabasePath()));
    } else {
        if (!QFileInfo::exists(noteDbPath) || !QFileInfo(noteDbPath).isFile()) {
            db.createNoteDb(noteDbPath);
        }
        db.open(noteDbPath);
    }
    init();
}

void MainWindow::getDatabasePath() {
    QString noteDbPath = QFileDialog::getSaveFileName(this, tr("Create Note database"), QDir::homePath(), tr("Sqlite (*.db)"));
    if (!noteDbPath.isEmpty()) {
        if (db.createNoteDb(noteDbPath)) {
            Config::getInstance().set(Config::getInstance().NOTE_DB_PATH, noteDbPath);
        }
    }
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
    this->setStyleSheet("QMainWindow { background-color: #252525; border: none; }");
    ui->splitter->setStretchFactor (0,0);
    ui->splitter->setStretchFactor (1,1);
    ui->splitter->setSizes(QList<int>() << 200 << 160000);
    ui->openedNotePath->setStyleSheet("color: white;");
    ui->fileList->setWordWrap(true);
    ui->noteText->setAcceptDrops(false);
    setAcceptDrops(true);

#ifdef Q_OS_DARWIN
    ui->fileList->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->searchText->setAttribute(Qt::WA_MacShowFocusRect, false);
#endif

    // changing font size is required to fix the inconsistent
    // word wrapping/ellipsis of fileList items
    QFont font = ui->fileList->font();
    font.setPointSize(13);
    ui->fileList->setFont(font);

    setNoteStyles();

    noteSaved = true;
    showWordCount = false;
    changeCount = 0;

    this->noteList = db.getRecents();
    resetNoteList();
#ifdef Q_OS_DARWIN
    helpFilePath = QApplication::applicationDirPath() + "/../Resources/help.md";
#elif Q_OS_WIN
    QString helpFilePath = "help.md";
#else
    QString helpFilePath = "~/.QuickNote/help.md";
#endif
    openHelpFile(helpFilePath);
}

void MainWindow::setNoteStyles() {
    styles = "QPlainTextEdit { padding: 5% 5% 0 5%; color: " + Config::getInstance().get(Config::getInstance().FOREGROUND) + "; background-color: " + Config::getInstance().get(Config::getInstance().BACKGROUND) + "; border:none; }";
    ui->noteText->setStyleSheet(styles);
    if (!Config::getInstance().get(Config::getInstance().FONT).isEmpty()) {
        QFont font;
        font.fromString(Config::getInstance().get(Config::getInstance().FONT));
        ui->noteText->setFont(font);
    }
    highlighter = new Highlighter(this);
    highlighter->setDocument(ui->noteText->document());
    highlighter->rehighlight();
    isConfigDialogActive = false;
}

void MainWindow::openHelpFile(QString &filePath) {
    bool fileExists = QFileInfo::exists(filePath) && QFileInfo(filePath).isFile();

    if (!fileExists) {
        return;
    }

    ui->noteText->setPlainText(QString::fromStdString(Helpers::getFileContent(filePath.toStdString())));
    isHelpFile = true;
#ifdef Q_OS_DARWIN
    setWindowModified(false);
#endif
}

bool MainWindow::noteSavePrompt() {
    QMessageBox msgBox;
    msgBox.setText("Unsaved Note");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Save) {
        if (!saveNote())
            return false;
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

bool MainWindow::saveNote() {
    std::string output = ui->noteText->toPlainText().toUtf8().constData();
    try {
        if (openedNote) {
            db.save(noteId, output);
        } else {
            noteId = db.save(output);
            openedNote = true;
        }
        noteSaved = true;
    } catch (std::exception& ex) {
        return false;
    }

#ifdef Q_OS_DARWIN
    setWindowModified(false);
#endif

    try {
        Tokenizer tokenizer;
        tokenizer.tokenize(output);
        tagArr = tokenizer.getTags();
        db.deleteTags(noteId);
        db.addTags(noteId, tagArr);
        this->noteList = db.getRecents();
        resetNoteList();
    } catch (std::exception& ex) {
        noteSaved = false;
        displayMessage("There was an error! please try again.");
        return false;
    }

    return true;
}

void MainWindow::resetNoteList() {
    ui->fileListOptions->blockSignals(true);
    ui->fileList->clear();
    ui->fileListOptions->clear();
    ui->fileListOptions->addItem("Recent Notes");
    ui->fileListOptions->addItem("All Notes");

    QVector<QString> tags = db.getTags();

    setNoteList();

    if (tags.size() > 0) {
        QStringList tagList = tags.toList();
        tagList.removeDuplicates();
        ui->fileListOptions->addItems(tagList);
    }

    ui->fileListOptions->blockSignals(false);
}

void MainWindow::setNoteList() {
    if (!this->noteList.empty()) {
        ui->fileList->clear();
        for (auto& path : this->noteList) {
            ui->fileList->addItem(path.at(1));
        }
    }
}

void MainWindow::reverseTab() {
    QTextCursor tempCursor = ui->noteText->textCursor();
    QString line = tempCursor.block().text();
    if (Helpers::checkListItem(line) && Helpers::getSpaceCount(line) >= 4) {
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
    if (openedNote && !noteSaved && !isHelpFile) {
        if (!noteSavePrompt())
            return;
    }
    noteSaved = false;
    openedNote = false;
    ui->openedNotePath->setText("");
    ui->noteText->setPlainText("# title");
    changeCount = 0;
    noteId = -1;
    isHelpFile = false;
    ui->fileList->setCurrentRow(-1);
    ui->fileList->selectionModel()->clearSelection();
}

void MainWindow::on_actionSave_triggered() {
    if (!isHelpFile && !noteSaved)
        saveNote();
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
    std::string selection = ui->noteText->textCursor().selection().toPlainText().toStdString();
    ui->noteText->insertPlainText(QString::fromStdString(Helpers::makeList("unordered", selection)));
}

void MainWindow::on_actionMake_Ordered_List_triggered() {
    std::string selection = ui->noteText->textCursor().selection().toPlainText().toStdString();
    ui->noteText->insertPlainText(QString::fromStdString(Helpers::makeList("ordered", selection)));
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

    std::string htmlSavePath = QFileDialog::getSaveFileName(this, tr("Export HTML File"), path, tr("HTML (*.html)")).toStdString();
    std::string htmlContent = tokenizer.getHTML();
    Helpers::exportHTML(htmlSavePath, htmlContent);
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
            this->noteList = db.getNotes();
        } else {
            this->noteList = db.getRecents();
        }
    } else {
        this->noteList = db.getNotesByTag(tag);
    }

    ui->searchText->blockSignals(true);
    ui->searchText->clear();
    ui->searchText->blockSignals(false);

    setNoteList();
}

void MainWindow::on_fileList_itemClicked(QListWidgetItem *item) {
    int note = this->noteList.at(ui->fileList->currentRow()).at(0).toInt();
    if (note == noteId && !isHelpFile) {
        return;
    }

    if (!noteSaved && !isHelpFile) {
        if (!noteSavePrompt()) {
            return;
        }
    }

    QString content = db.getNote(note);
    ui->noteText->setPlainText(content);

    noteSaved = true;
    openedNote = true;
    changeCount = 0;
    noteId = note;
    isHelpFile = false;

    #ifdef Q_OS_DARWIN
        setWindowModified(false);
    #endif
}

void MainWindow::on_noteText_textChanged() {
    changeCount += 1;
    noteSaved = (openedNote) ? (changeCount == 1) ? true : false : false;
#ifdef Q_OS_DARWIN
    if (!noteSaved && !isHelpFile)
        setWindowModified(true);
#endif

    if (showWordCount) {
        QString content = ui->noteText->toPlainText();
        QString wordCountText = "Word Count: " + Helpers::getWordCount(content);
        ui->openedNotePath->setText(wordCountText);
    }

    QTextBlock textBlock = ui->noteText->textCursor().block();    

    if (textBlock.text().length() == 0) {
        QString prevLine = textBlock.previous().text();
        if (Helpers::checkListItem(prevLine)) {
            int spaceCount = Helpers::getSpaceCount(prevLine);
            QString newLine = QString("").leftJustified(spaceCount, ' ');

            if (Helpers::checkUnorderedListItem(prevLine)) {
                newLine += "* ";
            } else {
                newLine += Helpers::getNextNumber(prevLine) + ". ";
            }

            ui->noteText->textCursor().insertText(newLine);
        }
    }
}

void MainWindow::closeEvent (QCloseEvent *event) {
    if (!noteSaved && !isHelpFile) {
        if (!noteSavePrompt())
            event->ignore();
    }
}

void MainWindow::on_actionShow_Word_Count_triggered() {
    this->showWordCount = (!this->showWordCount);
    QString text = "";
    if (this->showWordCount) {
        QString content = ui->noteText->toPlainText();
        text = "Word Count: " + Helpers::getWordCount(content);
    }
    ui->openedNotePath->setText(text);
}

void MainWindow::on_actionSupported_Markdown_triggered() {
    if (!noteSaved && !isHelpFile) {
        if (!noteSavePrompt())
            return;
    }
    openHelpFile(helpFilePath);
}

void MainWindow::on_actionDelete_triggered() {
    QListWidgetItem *currentItem = ui->fileList->currentItem();
    if (currentItem == NULL) {
        return;
    }

    QString noteTitle = currentItem->text();

    QMessageBox msgBox;
    msgBox.setText("Delete " + noteTitle);
    msgBox.setStyleSheet("QLabel{min-width: 350px;}");
    msgBox.setInformativeText("Do you want to delete \"" + noteTitle + "\"");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        if (db.deleteNote(noteId)) {
            displayMessage("Note " + noteTitle + " deleted...");
            openHelpFile(helpFilePath);
            noteList = db.getRecents();
            resetNoteList();
        } else {
            displayMessage("Unable to delete " + noteTitle);
        }
    }
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox msgBox;
    msgBox.setText("QuickNote v5.0");
    msgBox.setInformativeText("QuickNote is a simple note app with markdown support.\nhttps://www.github.com/dhamith93/QuickNote");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    QSpacerItem* horizontalSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.exec();
}

void MainWindow::on_searchText_textChanged(const QString &arg1) {
    QString key = arg1.trimmed();

    if (key.isEmpty()) {
        return;
    }

    this->noteList = db.search(key);
    this->resetNoteList();
}

void MainWindow::on_actionDisplay_Options_triggered() {
    if (isConfigDialogActive)
        return;

    isConfigDialogActive = true;
    DisplayConfigDialog *configDialog = new DisplayConfigDialog();
    configDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(configDialog, SIGNAL(destroyed(QObject*)), SLOT(setNoteStyles()));
    configDialog->show();
}
