#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newnotewindow.h"
#include <QFileDialog>
#include <QtWidgets>
#include <QCloseEvent>
#include "database.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    Database db;
    QVector<QString> recentPaths = db.getRecents();
    if (recentPaths.size() > 0) {
        for (auto& path : recentPaths) {
            ui->listRecent->addItem(path);
        }
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_btnNewNote_clicked() {
    if (webEnginePreviewChecked) {
        newWebEngineNote = new NewWebEngineNoteWindow(this, "");
        newWebEngineNote->show();
    } else {
        newNote = new NewNoteWindow(this, "");
        newNote->show();
    }
    this->hide();
}

void MainWindow::on_btnOpenNote_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Markdown (*.md)"));
    if (fileName.size() > 0) {
        if (webEnginePreviewChecked) {
            newWebEngineNote = new NewWebEngineNoteWindow(this, fileName.toUtf8().constData());
            newWebEngineNote->show();
        } else {
            newNote = new NewNoteWindow(this, fileName.toUtf8().constData());
            newNote->show();
        }
        this->hide();
    }
}

void MainWindow::closeEvent (QCloseEvent *event) {

}

void MainWindow::on_listRecent_itemClicked(QListWidgetItem *item) {
    try {
        if (webEnginePreviewChecked) {
            newWebEngineNote = new NewWebEngineNoteWindow(this, item->text().toUtf8().constData());
            newWebEngineNote->show();
        } else {
            newNote = new NewNoteWindow(this, item->text().toUtf8().constData());
            newNote->show();
        }
        this->hide();
    } catch (std::exception ex) {
        QMessageBox msgBox;
        msgBox.setText("Can't find the file!");
        msgBox.exec();
        Database db;
        db.deletePath(item->text().toUtf8().constData());
        QVector<QString> recentPaths = db.getRecents();
        ui->listRecent->clear();
        if (recentPaths.size() > 0) {
            for (auto& path : recentPaths) {
                ui->listRecent->addItem(path);
            }
        }
    }
}

void MainWindow::on_chkWebEnginePreview_stateChanged(int arg1) {
    if (ui->chkWebEnginePreview->isChecked()) {
        webEnginePreviewChecked = true;
        return;
    }
    webEnginePreviewChecked = false;
}
