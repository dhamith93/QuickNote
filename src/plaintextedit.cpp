#include "headers/plaintextedit.h"
#include <QDebug>

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) { }

void PlainTextEdit::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Tab) {
        event->ignore();
        this->textCursor().insertText("    ");
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}
