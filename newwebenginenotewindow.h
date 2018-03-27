#ifndef NEWWEBENGINENOTEWINDOW_H
#define NEWWEBENGINENOTEWINDOW_H

#include <QMainWindow>

namespace Ui {
    class NewWebEngineNoteWindow;
}

class NewWebEngineNoteWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit NewWebEngineNoteWindow(QWidget *parent = 0);
        explicit NewWebEngineNoteWindow(QWidget *parent = 0, std::string filePath = "");
        ~NewWebEngineNoteWindow();

    private slots:
        void on_txtInput_textChanged();

        void setText(std::string &content);

        void saveFile();

        void saveFile(std::string path);

        void saveHtml();

        void closeEvent (QCloseEvent *event);

        void on_actionSave_triggered();

        void on_actionHTML_triggered();

        void on_actionDOCX_triggered();

        void on_actionChange_Font_triggered();

        void on_actionPreview_changed();

    private:
        Ui::NewWebEngineNoteWindow *ui;
        QString fileName;
        bool fileSaved = false;
        bool fromOpen = false;
        int changeCount = 0;
};

#endif // NEWWEBENGINENOTEWINDOW_H
