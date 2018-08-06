#include "headers/plaintextedit.h"

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) { }

void PlainTextEdit::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Tab) {
        event->ignore();
        this->textCursor().insertText("    ");
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}

void PlainTextEdit::focusOutEvent(QFocusEvent *event) {
    if (event->reason() == Qt::TabFocusReason) {
        this->setFocus();
        return;
    }
    QPlainTextEdit::focusOutEvent(event);
}
