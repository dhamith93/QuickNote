#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newnotewindow.h"
#include <QFileDialog>
#include <QtWidgets>
#include <QCloseEvent>
#include <iostream>
#include "database.cpp"

#ifdef _WIN32
    #define DBPATH "notePathDB.db"
#else
    #define DBPATH "../../../notePathDB.db"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    Database db(DBPATH);
    QVector<QString> recentPaths = db.getRecents();
    if (recentPaths.size() > 0) {
        for (auto& path : recentPaths) {
            ui->listRecent->addItem(path);
        }
    } else {
        ui->listRecent->addItem("No Recent notes...");
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_btnNewNote_clicked() {
    newNote = new NewNoteWindow(this, "");
    newNote->show();
}

void MainWindow::on_btnOpenNote_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Markdown (*.md)"));
    if (fileName.size() > 0) {
        newNote = new NewNoteWindow(this, fileName.toUtf8().constData());
        newNote->show();
    }
}

void MainWindow::closeEvent (QCloseEvent *event) {
    if (newNote != NULL) {
        if (newNote->isVisible()) {
            QMessageBox msgBox;
            msgBox.setText("You have editor window(s) open. Close them first.");
            msgBox.exec();
            event->ignore();
        }
    }
}

void MainWindow::on_listRecent_itemClicked(QListWidgetItem *item) {
    try {
        newNote = new NewNoteWindow(this, item->text().toUtf8().constData());
        newNote->show();
    } catch (exception ex) {
        QMessageBox msgBox;
        msgBox.setText("Can't find the file!");
        msgBox.exec();
        Database db(DBPATH);
        db.deletePath(item->text().toUtf8().constData());
        QVector<QString> recentPaths = db.getRecents();
        ui->listRecent->clear();
        if (recentPaths.size() > 0) {
            for (auto& path : recentPaths) {
                ui->listRecent->addItem(path);
            }
        } else {
            ui->listRecent->addItem("No Recent notes...");
        }
    }
}
