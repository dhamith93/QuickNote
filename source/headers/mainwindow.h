#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileInfo>
#include <QListWidgetItem>
#include "newnotewindow.h"
#include "database.h"
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private slots:
        void on_btnNewNote_clicked();

        void on_btnOpenNote_clicked();

        void on_listRecent_itemClicked(QListWidgetItem *item);

        void on_cmbTags_currentTextChanged(const QString &arg1);

        void on_listNotesByTag_itemClicked(QListWidgetItem *item);

        bool event(QEvent *e);

    private:
        Ui::MainWindow *ui;
        NewNoteWindow *newNote;
        Database db;
};

#endif // MAINWINDOW_H
