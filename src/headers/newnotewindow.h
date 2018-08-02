#ifndef NEWNOTEWINDOW_H
#define NEWNOTEWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QKeyEvent>
#include <vector>
#include <string>
#include "highlighter.h"
#include "database.h"

namespace Ui {
    class NewNoteWindow;
}

class NewNoteWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit NewNoteWindow(QWidget *parent = 0);
        explicit NewNoteWindow(QWidget *parent = 0, std::string filePath = "");
        ~NewNoteWindow();

    private slots:
        void on_actionSave_triggered();
        void on_noteText_textChanged();
        void on_actionHTML_triggered();
        void on_actionCopy_Selection_As_HTML_triggered();

        void on_actionChange_Font_triggered();

    private:
        Ui::NewNoteWindow *ui;
        Highlighter *highlighter;
        Database db;
        QString fileName;
        std::vector<std::string> tagArr;
        bool fileSaved;
        bool openedFile;
        int changeCount;

        void init();
        void saveFile();
        void closeEvent (QCloseEvent *event);        
        std::string getFileContent(std::string path);
};

#endif // NEWNOTEWINDOW_H
