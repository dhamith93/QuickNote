#ifndef NEWNOTEWINDOW_H
#define NEWNOTEWINDOW_H

#include <QMainWindow>

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
        void on_txtInput_textChanged();

        void on_txtInput_cursorPositionChanged();

        void setText(std::string &content);

        void setScrollPosition();

        void saveFile();

        void saveFile(std::string path);

        void saveHtml();

        void closeEvent (QCloseEvent *event);

        void on_actionSave_triggered();

        void on_actionHTML_triggered();

        void on_actionDOCX_triggered();

    private:
        Ui::NewNoteWindow *ui;
        QString fileName;
        bool fileSaved = false;
        bool fromOpen = false;
        int changeCount = 0;
};

#endif // NEWNOTEWINDOW_H
