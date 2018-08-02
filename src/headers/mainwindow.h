#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
        void on_newNoteBtn_clicked();
        bool event(QEvent *e);

        void on_fileListOptions_currentTextChanged(const QString &arg1);

        void on_fileList_itemClicked(QListWidgetItem *item);

        void on_actionOpen_triggered();

    private:
        Ui::MainWindow *ui;
        NewNoteWindow *newNoteWindow;
        Database db;
};

#endif // MAINWINDOW_H
