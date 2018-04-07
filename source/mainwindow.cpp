#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/newnotewindow.h"
#include <QFileDialog>
#include <QtWidgets>
#include <QCloseEvent>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
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

void MainWindow::on_listRecent_itemClicked(QListWidgetItem *item) {
    std::string path = item->text().toUtf8().constData();
    try {
        newNote = new NewNoteWindow(NULL, path);
        newNote->show();
    } catch (std::exception ex) {
        QMessageBox msgBox;
        msgBox.setText("Can't find the file!");
        msgBox.exec();
        db.deletePath(QString::fromStdString(path));
        QVector<QString> recentPaths = db.getRecents();
        ui->listRecent->clear();
        if (recentPaths.size() > 0) {
            for (auto& path : recentPaths) {
                ui->listRecent->addItem(path);
            }
        }
    }
}

void MainWindow::on_cmbTags_currentTextChanged(const QString &arg1) {
    std::string tag = arg1.toUtf8().constData();
    if (tag != "Select a tag") {
        QVector<QString> notesBytag = db.getNotesByTag(tag);
        if (notesBytag.size() > 0) {
            ui->listNotesByTag->clear();
            for (auto& note : notesBytag) {
                ui->listNotesByTag->addItem(note);
            }
        }
    } else {
        ui->listNotesByTag->clear();
    }
}

void MainWindow::on_listNotesByTag_itemClicked(QListWidgetItem *item) {
    std::string path = item->text().toUtf8().constData();
    try {
        newNote = new NewNoteWindow(NULL, path);
        newNote->show();
    } catch (std::exception ex) {
        QMessageBox msgBox;
        msgBox.setText("Can't find the file!");
        msgBox.exec();
        db.deleteTaggedPath(QString::fromStdString(path));
        ui->cmbTags->clear();
        QVector<QString> tags = db.getTags();
        if (tags.size() > 0) {
            QStringList tagList = tags.toList();
            tagList.removeDuplicates();
            ui->cmbTags->addItem("Select a tag");
            ui->cmbTags->addItems(tagList);
        }
        ui->listNotesByTag->clear();
    }
}

bool MainWindow::event(QEvent *e) {
    if (e->type() == QEvent::WindowActivate) {
        ui->listRecent->clear();
        ui->cmbTags->clear();
        ui->listNotesByTag->clear();
        QVector<QString> recentPaths = db.getRecents();
        QVector<QString> tags = db.getTags();
        if (recentPaths.size() > 0) {
            for (auto& path : recentPaths) {
                ui->listRecent->addItem(path);
            }
        }
        if (tags.size() > 0) {
            QStringList tagList = tags.toList();
            tagList.removeDuplicates();
            ui->cmbTags->addItem("Select a tag");
            ui->cmbTags->addItems(tagList);
        }
    }
    return QWidget::event(e);
}
