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
        void on_noteText_textChanged();
        void on_actionSave_triggered();
        void on_actionChange_Font_triggered();
        void on_actionCopy_selection_as_HTML_triggered();
        void on_actionInsert_Table_triggered();
        void on_actionMake_Unordered_List_triggered();
        void on_actionMake_Ordered_List_triggered();
        void on_actionExport_HTML_triggered();
        void on_actionEncrypt_note_triggered();
        void on_actionDecrypt_Note_triggered();
        void on_actionLight_triggered();
        void on_actionDark_triggered();
        void on_actionShow_Word_Count_triggered();
        void on_actionAbout_triggered();

    protected:

    private:
        Ui::MainWindow *ui;
        Highlighter *highlighter;
        Database db;
        QString fileName;
        QString darkStyles;
        QString lightStyles;
        std::vector<std::string> tagArr;
        bool noteSaved;
        bool showWordCount;
        int changeCount;
        bool isHelpFile;
        std::string displayMode;
        QVector<QVector<QString>> paths;

        int noteId;
        bool openedNote;

        void closeEvent(QCloseEvent *event);
        bool eventFilter(QObject *watched, QEvent *event);
        void init();
        void resetNoteList();
        void setNoteList();
        void openHelpFile(QString &filePath);
        bool noteSavePrompt();
        bool saveNote();
        void setDisplayModeLight();
        void setDisplayModeDark();
        void reverseTab();
        void displayMessage(QString message);

        Q_INVOKABLE void getDatabasePath();
};

#endif // MAINWINDOW_H
