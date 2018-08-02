#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QPlainTextEdit>


class PlainTextEdit : public QPlainTextEdit {
    public:
        PlainTextEdit(QWidget *parent = 0);
    protected:
        virtual void keyPressEvent(QKeyEvent *event);
};

#endif // PLAINTEXTEDIT_H
