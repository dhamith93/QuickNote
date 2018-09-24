#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/database.h"
#include "headers/highlighter.h"
#include "headers/encryption.h"
#include "mdLite/token.h"
#include "mdLite/tokenizer.h"
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextDocumentFragment>
#include <sstream>
#include <fstream>

#ifdef Q_OS_DARWIN
    #include "headers/macosuihandler.h"
#endif

QString enc = "";

MainWindow::MainWindow(QWidget *parent) :
                QMainWindow(parent),
                ui(new Ui::MainWindow) {
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::init() {    
    darkStyles = "QPlainTextEdit { padding: 5% 5% 0 5%; color: white; background-color: #252525; border:none; }";
    lightStyles = "QPlainTextEdit { padding: 5% 5% 0 5%; color: #454545; background-color: #FAFAFA; border:none; }";

    this->setStyleSheet("QMainWindow { background-color: #505050; border: none; }");
    ui->splitter->setStretchFactor (0,0);
    ui->splitter->setStretchFactor (1,1);
    ui->noteText->setStyleSheet(darkStyles);
    ui->openedNotePath->setStyleSheet("color: white;");
    ui->actionDark->setChecked(true);
    ui->fileList->setWordWrap(true);

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
            fileName = QFileDialog::getSaveFileName(this, tr("Save Note"), QDir::homePath(), tr("Markdown (*.md)"));
            if (fileName.isEmpty())
                return false;
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
        QMessageBox msgBox;
        msgBox.setText("There was an error! please try again.");
        msgBox.exec();
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
    QVector<QString> recentPaths = db.getRecents();
    QVector<QString> tags = db.getTags();
    if (recentPaths.size() > 0) {
        for (auto& path : recentPaths)
            ui->fileList->addItem(path);
    }
    if (tags.size() > 0) {
        QStringList tagList = tags.toList();
        tagList.removeDuplicates();
        ui->fileListOptions->addItems(tagList);
    }
    ui->fileListOptions->blockSignals(false);
}

void MainWindow::openedFileHelper() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Markdown (*.md)"));
    if (fileName.size() > 0) {
        openFile(fileName);
        resetFileList();
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

void MainWindow::on_actionCopy_selection_as_HTML_triggered() {
    QString selection = ui->noteText->textCursor().selection().toPlainText() + "\n";
    std::string text = selection.toStdString();
    Tokenizer tokenizer;
    tokenizer.tokenize(text);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::fromStdString(tokenizer.getHTML()));
}

void MainWindow::on_actionExport_HTML_triggered() {
    std::string text = ui->noteText->toPlainText().toStdString();
    Tokenizer tokenizer;
    tokenizer.tokenize(text);

    QString htmlSavePath = QFileDialog::getSaveFileName(this, tr("Export HTML File"), QDir::homePath(), tr("HTML (*.html)"));
    std::ofstream out(htmlSavePath.toUtf8().constData());
    std::string headerPath = "html/header.html";
    std::string footerPath = "html/footer.html";

    #ifdef Q_OS_DARWIN
    headerPath = "../Resources/header.html";
    footerPath = "../Resources/footer.html";
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
        QMessageBox msgBox;
        msgBox.setText("Passphrase don't match!");
        msgBox.exec();
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
        QMessageBox msgBox;
        msgBox.setText("Something went wrong during encryption...");
        msgBox.exec();
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
        QMessageBox msgBox;
        msgBox.setText("It seems like your note is not encrypted...");
        msgBox.exec();
    }
}

void MainWindow::on_fileListOptions_currentTextChanged(const QString &arg1) {
    std::string tag = arg1.toUtf8().constData();
    if (tag == "Recent Notes" || tag == "All Notes") {
        ui->fileList->clear();
        QVector<QString> paths;
        if (tag == "All Notes") {
            paths = db.getNotes();
        } else {
            paths = db.getRecents();
        }
        if (paths.size() > 0) {
            for (auto& path : paths)
                ui->fileList->addItem(path);
        }
    } else {
        QVector<QString> notesBytag = db.getNotesByTag(tag);
        if (notesBytag.size() > 0) {
            ui->fileList->clear();
            for (auto& note : notesBytag)
                ui->fileList->addItem(note);
        }
    }
}

void MainWindow::on_fileList_itemClicked(QListWidgetItem *item) {
    QString filePath = item->text().toUtf8().constData();
    if (fileSaved && filePath == fileName)
        return;
    openFile(filePath);
    if (ui->fileListOptions->currentText() == "Recent Notes") {
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
