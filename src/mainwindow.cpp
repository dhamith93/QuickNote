#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/newnotewindow.h"
#include "headers/database.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>


#ifdef Q_OS_DARWIN
    #include "headers/macosuihandler.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
                QMainWindow(parent),
                ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setStyleSheet("QMainWindow { background-color: #252525; border: none; }");
    setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_newNoteBtn_clicked() {
    newNoteWindow = new NewNoteWindow(NULL, "");
    newNoteWindow->show();
    #ifdef Q_OS_DARWIN
    MacOSUIHandler::setTitleBar(*newNoteWindow);
    #endif
}

void MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Markdown (*.md)"));
    if (fileName.size() > 0) {
        newNoteWindow = new NewNoteWindow(NULL, fileName.toUtf8().constData());
        newNoteWindow->show();
        #ifdef Q_OS_DARWIN
        MacOSUIHandler::setTitleBar(*newNoteWindow);
        #endif
    }
}

void MainWindow::on_fileListOptions_currentTextChanged(const QString &arg1) {
    std::string tag = arg1.toUtf8().constData();
    if (tag == "Recent Notes") {
        ui->fileList->clear();
        QVector<QString> recentPaths = db.getRecents();
        if (recentPaths.size() > 0) {
            for (auto& path : recentPaths)
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
    QString path = item->text().toUtf8().constData();
    bool fileExists = QFileInfo::exists(path) && QFileInfo(path).isFile();
    if (fileExists) {
        newNoteWindow = new NewNoteWindow(NULL, path.toStdString());
        newNoteWindow->show();
        #ifdef Q_OS_DARWIN
        MacOSUIHandler::setTitleBar(*newNoteWindow);
        #endif
    } else {
        QMessageBox msgBox;
        msgBox.setText("Can't find the file! Maybe it was deleted or moved?\n");
        msgBox.exec();
    }
}

bool MainWindow::event(QEvent *e) {
    if (e->type() == QEvent::WindowActivate) {
        ui->fileListOptions->blockSignals(true);
        ui->fileList->clear();
        ui->fileListOptions->clear();
        ui->fileListOptions->addItem("Recent Notes");
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
    return QWidget::event(e);
}
