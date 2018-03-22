#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newnotewindow.h"
#include <QFileDialog>
#include <QtWidgets>
#include <QCloseEvent>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
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
