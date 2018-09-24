#include "headers/plaintextedit.h"
#include "QTextBlock"
#include "QRegularExpression"

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) { }

bool PlainTextEdit::checkListItem(QString &line) {
    QRegularExpression regex1("^\\s*\\*\\s");
    QRegularExpression regex2("^\\s*\\d*\\.\\s");
    return (regex1.match(line).hasMatch() || regex2.match(line).hasMatch());
}

bool PlainTextEdit::checkEmptyListItem(QString &line) {
    QRegularExpression regex1("^\\s*\\*([[:blank:]]){1,}$");
    QRegularExpression regex2("^\\s*\\d*\\.([[:blank:]]){1,}$");
    return (regex1.match(line).hasMatch() || regex2.match(line).hasMatch());
}

QTextCursor PlainTextEdit::getModifiedTextCursor(QTextCursor textCursorIn, QString text) {
    QTextCursor tempCursor = textCursorIn;
    tempCursor.movePosition(QTextCursor::StartOfLine);
    tempCursor.movePosition(QTextCursor::EndOfLine);
    tempCursor.select(QTextCursor::LineUnderCursor);
    tempCursor.removeSelectedText();
    tempCursor.insertText(text);
    return tempCursor;
}

void PlainTextEdit::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Tab) {
        event->ignore();
        QTextCursor tempCursor = this->textCursor();
        QTextBlock block = this->textCursor().block();
        QString str = block.text();
        bool listItem = checkListItem(str);
        int pos = tempCursor.positionInBlock();

        if (listItem)
            tempCursor.movePosition(QTextCursor::StartOfLine);

        tempCursor.insertText("    ");

        if (listItem)
            tempCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);

        this->setTextCursor(tempCursor);

        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextBlock block = this->textCursor().block();
        QString str = block.text();

        if (checkEmptyListItem(str)) {
            event->ignore();
            this->blockSignals(true);
            this->setTextCursor(getModifiedTextCursor(this->textCursor(), "\n"));
            this->blockSignals(false);
            return;
        }
    }

    if (event->key() == Qt::Key_Backspace) {
        QTextBlock block = this->textCursor().block();
        QString str = block.previous().text();
        if (block.text().length() == 1 && checkListItem(str)) {
            event->ignore();
            this->blockSignals(true);
            this->setTextCursor(getModifiedTextCursor(this->textCursor(), ""));
            this->blockSignals(false);
            return;
        }
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
