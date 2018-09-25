#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QTextCursor>


class PlainTextEdit : public QPlainTextEdit {
    public:
        PlainTextEdit(QWidget *parent = 0);
    private:
        bool checkListItem(QString &line);
        bool checkEmptyListItem(QString &line);
        QTextCursor getModifiedTextCursor(QString text);
    protected:
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void focusOutEvent(QFocusEvent* e);        
};

#endif // PLAINTEXTEDIT_H
