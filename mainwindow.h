#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileInfo>
#include "newnotewindow.h"
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

    private:
        Ui::MainWindow *ui;
        NewNoteWindow *newNote = NULL;
};

#endif // MAINWINDOW_H
