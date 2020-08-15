#ifndef DISPLAYCONFIGDIALOG_H
#define DISPLAYCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
    class DisplayConfigDialog;
}

class DisplayConfigDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DisplayConfigDialog(QWidget *parent = 0);
        ~DisplayConfigDialog();

    private slots:
        void init();
        QColor pickColor(QColor &color);
        void on_btnReset_clicked();
        void on_btnFontChange_clicked();
        void on_btnCommentsColorChange_clicked();
        void on_btnFontColorChange_clicked();
        void on_btnBgColorChange_clicked();
        void on_btnItalicColorChange_clicked();
        void on_btnBoldColorChange_clicked();
        void on_btnSThroughColorColor_clicked();
        void on_btnHeadersColorChange_clicked();
        void on_btnBQuotesColorChange_clicked();
        void on_btnCodeColorChange_clicked();
        void on_btnSymbolsColorChange_clicked();
        void on_btnTagsColorChange_clicked();

    private:
        Ui::DisplayConfigDialog *ui;
};

#endif // DISPLAYCONFIGDIALOG_H
