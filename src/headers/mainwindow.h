#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QCloseEvent>
#include "highlighter.h"
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
        void on_fileListOptions_currentTextChanged(const QString &arg1);
        void on_fileList_itemClicked(QListWidgetItem *item);
        void on_actionOpen_triggered();
        void on_noteText_textChanged();
        void on_openNoteBtn_clicked();
        void on_actionSave_triggered();
        void on_actionChange_Font_triggered();
        void on_actionCopy_selection_as_HTML_triggered();
        void on_actionExport_HTML_triggered();
        void on_actionEncrypt_note_triggered();
        void on_actionDecrypt_Note_triggered();
        void on_actionLight_triggered();
        void on_actionDark_triggered();
        void on_actionShow_Word_Count_triggered();

        void on_actionAbout_triggered();

    private:
        Ui::MainWindow *ui;
        Highlighter *highlighter;
        Database db;
        QString fileName;
        QString darkStyles;
        QString lightStyles;
        std::vector<std::string> tagArr;
        bool fileSaved;
        bool openedFile;
        bool showWordCount;
        int changeCount;
        std::string displayMode;

        void closeEvent(QCloseEvent *event);
        void init();
        void resetFileList();        
        void openFile(QString &filePath);
        bool fileSavePromt();
        bool saveFile();
        std::string getFileContent(std::string path);
        void openedFileHelper();
        void setDisplayModeLight();
        void setDisplayModeDark();
        QString getWordCount();
        bool previousLineIsListItem(QString &line);
        bool unorderedListItem(QString &line);
        int getSpaceCount(QString &line);
        QString getNextNumber(QString &line);
};

#endif // MAINWINDOW_H
