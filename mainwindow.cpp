#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newnotewindow.h"
#include <QFileDialog>
#include <QtWidgets>
#include <QCloseEvent>

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
    newNote = new NewNoteWindow(NULL, "");
    newNote->show();
}

void MainWindow::on_btnOpenNote_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Markdown (*.md)"));
    if (fileName.size() > 0) {
        newNote = new NewNoteWindow(NULL, fileName.toUtf8().constData());
        newNote->show();
    }
}

void MainWindow::closeEvent (QCloseEvent *event) {

}

void MainWindow::on_listRecent_itemClicked(QListWidgetItem *item) {
    try {
        newNote = new NewNoteWindow(NULL, item->text().toUtf8().constData());
        newNote->show();
    } catch (std::exception ex) {
        QMessageBox msgBox;
        msgBox.setText("Can't find the file!");
        msgBox.exec();
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
