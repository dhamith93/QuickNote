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

        void closeEvent (QCloseEvent *event);

        void on_listRecent_itemClicked(QListWidgetItem *item);

    private:
        Ui::MainWindow *ui;
        NewNoteWindow *newNote;
        Database db;
};

#endif // MAINWINDOW_H
