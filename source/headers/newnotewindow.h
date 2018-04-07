#ifndef NEWNOTEWINDOW_H
#define NEWNOTEWINDOW_H

#include <QMainWindow>
#include <vector>
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
        void on_txtInput_textChanged();

        void on_txtInput_cursorPositionChanged();

        void setText(std::string &content);

        void setScrollPosition();

        void saveFile();

        void saveFile(std::string path);

        void saveHtml();

        QString tempSaveHtml();

        void closeEvent (QCloseEvent *event);

        void on_actionSave_triggered();

        void on_actionHTML_triggered();

        void on_actionDOCX_triggered();

        void on_actionChange_Font_triggered();

        void on_actionPreview_changed();

        void on_actionPretty_Preview_triggered();

        void on_actionSet_Background_Color_triggered();

        void on_actionSet_Font_Color_triggered();

        void setColor(QColor color1, QString type);

        void on_actionPDF_triggered();

    private:
        Ui::NewNoteWindow *ui;
        QString fileName;
        Database db;
        bool fileSaved = false;
        bool fromOpen = false;
        int changeCount = 0;
        std::string tagLine;
        std::vector<std::string> tagArr;
};

#endif // NEWNOTEWINDOW_H
