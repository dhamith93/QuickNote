#include "headers/newnotewindow.h"
#include "headers/plaintextedit.h"
#include "ui_newnotewindow.h"
#include "headers/highlighter.h"
#include "mdLite/tokenizer.h"
#include <QCloseEvent>
#include <QClipboard>
#include <QFileDialog>
#include <QFontDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTextDocumentFragment>
#include <QDebug>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

NewNoteWindow::NewNoteWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::NewNoteWindow) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    init();
}

NewNoteWindow::NewNoteWindow(QWidget *parent, std::string filePath) :
    QMainWindow(parent),
    ui(new Ui::NewNoteWindow) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    init();
    if (db.fontConfigExists()) {        
        QFont font;
        font.fromString(db.getFontConfig());        
        ui->noteText->setFont(font);
    }
    if (filePath != "") {
        openedFile = true;
        fileSaved = true;
        fileName = QString::fromStdString(filePath);
        setWindowTitle(fileName);
        std::ifstream file(filePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();
        ui->noteText->setPlainText(QString::fromStdString(content));
        if (!db.recentNoteExists(fileName)) {
            if (db.checkRowCountEq(20)) {
                db.deleteOldest();
            }
            db.insertNote(fileName);
        }
    } else {
        #ifdef Q_OS_DARWIN
        setWindowModified(true); // Sets 'content modified' on macos. It prompts save window on exit
        #endif
    }
}

void NewNoteWindow::init() {
    this->setStyleSheet("QMainWindow, QStatusBar { background-color: #252525; border: none; }");
    ui->noteText->setStyleSheet("QPlainTextEdit { padding: 5% 25% 0 25%; color: white; background-color: #252525; border:none; }");
    highlighter = new Highlighter(this);
    highlighter->setDocument(ui->noteText->document());
    ui->noteText->setStyleSheet("QPlainTextEdit { padding: 5% 25% 0 25%; color: white; background-color: #252525; border:none; }");
    ui->noteText->textCursor().blockFormat().setLineHeight(12, 1);
    fileSaved = false;
    openedFile = false;
    changeCount = 0;
}

void NewNoteWindow::saveFile() {
    if (!openedFile) {
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath(), tr("Markdown (*.md)"));
    }

    try {
        std::ofstream out(fileName.toUtf8().constData());
        std::string output = ui->noteText->toPlainText().toUtf8().constData();
        out << output;
        out.close();
        fileSaved = true;
        openedFile = true;
        setWindowModified(false);
        setWindowTitle(fileName);

        Tokenizer tokenizer;
        tokenizer.tokenize(output);
        tagArr = tokenizer.getTags();

        if (!db.recentNoteExists(fileName)) {
            if (db.checkRowCountEq(20)) {
                db.deleteOldest();
            }
            db.insertNote(fileName);
        }
        if (!db.taggedNoteExists(fileName)) {
            if (tagArr.size() > 0) {
                db.insertNoteWithTags(fileName, tagArr);
            }
        } else {
            db.deleteTags(db.getRowId(fileName.toUtf8().constData()));
            if (tagArr.size() > 0) {
                db.updateTags(fileName, tagArr);
            }
        }
    } catch (std::exception ex) {
        fileSaved = false;
        QMessageBox msgBox;
        msgBox.setText("There was an error! please try again.");
        msgBox.exec();
    }
}

void NewNoteWindow::on_noteText_textChanged() {
    changeCount += 1;
    fileSaved = (openedFile) ? (changeCount == 1) ? true : false : false;
    #ifdef Q_OS_DARWIN
    if (!fileSaved) {
        setWindowModified(true);
    }
    #endif
}

void NewNoteWindow::on_actionSave_triggered() {
    if (!fileSaved)
        saveFile();
}

void NewNoteWindow::on_actionHTML_triggered() {
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


void NewNoteWindow::on_actionCopy_Selection_As_HTML_triggered() {
    QString selection = ui->noteText->textCursor().selection().toPlainText();
    std::string text = selection.toStdString();
    Tokenizer tokenizer;
    tokenizer.tokenize(text);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::fromStdString(tokenizer.getHTML()));
}

void NewNoteWindow::on_actionChange_Font_triggered() {
    bool changed;
    QFont font = QFontDialog::getFont(&changed);
    if (changed) {
        db.insertFontConfig(font.toString());
        ui->noteText->setFont(font);
    }
}

void NewNoteWindow::closeEvent (QCloseEvent *event) {
    if (!fileSaved) {
        QMessageBox msgBox;
        msgBox.setText("Unsaved Note");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Save) {
            event->ignore();
            saveFile();
            event->accept();
        } else if (ret == QMessageBox::Cancel) {
            event->ignore();
        } else {
            event->accept();
            this->close();
        }
    }
}

std::string NewNoteWindow::getFileContent(std::string path) {
    try {
        std::ifstream ifs(path);
        std::string output((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        ifs.close();

        return output;
    } catch(const std::exception& e) {

    }

    return "";
}

NewNoteWindow::~NewNoteWindow() {
    delete highlighter;
    delete ui;
}
