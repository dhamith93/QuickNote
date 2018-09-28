#include "headers/plaintextedit.h"
#include <QTextBlock>
#include <QRegularExpression>
#include <QTextDocumentFragment>

#include "QDebug"

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

QTextCursor PlainTextEdit::getModifiedTextCursor(QString text) {
    QTextCursor tempCursor = textCursor();
    tempCursor.movePosition(QTextCursor::StartOfLine);
    tempCursor.movePosition(QTextCursor::EndOfLine);
    tempCursor.select(QTextCursor::LineUnderCursor);
    tempCursor.removeSelectedText();
    tempCursor.insertText(text);
    return tempCursor;
}

void PlainTextEdit::keyPressEvent(QKeyEvent *event) {

#ifdef Q_OS_DARWIN

    // macos only
    // Override macos native HOME & END keypress events

    if (event->key() == Qt::Key_Home || event->key() == Qt::Key_End) {
        event->ignore();
        QTextCursor tempCursor = this->textCursor();
        if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            tempCursor.setPosition(tempCursor.position(), QTextCursor::KeepAnchor);
            if (event->key() == Qt::Key_Home) {
                tempCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            } else {
                tempCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            }
        } else {
            if (event->key() == Qt::Key_Home) {
                tempCursor.movePosition(QTextCursor::StartOfLine);
            } else {
                tempCursor.movePosition(QTextCursor::EndOfLine);
            }
        }
        setTextCursor(tempCursor);
        return;
    }

#endif

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
            this->setTextCursor(getModifiedTextCursor("\n"));
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
            this->setTextCursor(getModifiedTextCursor(""));
            this->blockSignals(false);
            return;
        }
    }

    // Autocomplete (, [, {, *, `, ', "
    if (event->key() == Qt::Key_BraceLeft || event->key() == Qt::Key_BracketLeft
            || event->key() == Qt::Key_ParenLeft || event->key() == Qt::Key_Apostrophe
            || event->key() == Qt::Key_QuoteDbl || event->key() == Qt::Key_Asterisk
            || event->key() == Qt::Key_QuoteLeft || event->key() == Qt::Key_AsciiTilde) {

        // if `*` pressed on an empty line (i.e. list item)
        // run the defualt keypress event
        if (event->key() == Qt::Key_Asterisk && this->textCursor().block().text().isEmpty()) {
            QPlainTextEdit::keyPressEvent(event);
            return;
        }

        QString str = "";

        switch (event->key()) {
            case Qt::Key_BraceLeft:
                str = "}";
                break;
            case Qt::Key_BracketLeft:
                str = "]";
                break;
            case Qt::Key_ParenLeft:
                str = ")";
                break;
            case Qt::Key_Apostrophe:
                str = "'";
                break;
            case Qt::Key_QuoteDbl:
                str = "\"";
                break;
            case Qt::Key_Asterisk:
                str = "*";
                break;
            case Qt::Key_QuoteLeft:
                str = "`";
                break;
            case Qt::Key_AsciiTilde:
                str = "~";
                break;
            default:
                break;
        }

        QTextCursor tempCursor = this->textCursor();
        if (tempCursor.hasSelection()) {
            int pos = tempCursor.selectionStart() + 1;
            QString out = event->text() + tempCursor.selection().toPlainText() + str;
            tempCursor.insertText(out);
            tempCursor.setPosition(pos, QTextCursor::MoveAnchor);
            tempCursor.setPosition((pos - 2) + out.length(), QTextCursor::KeepAnchor);
            this->setTextCursor(tempCursor);
            return; // this avoids adding extra key char into text
        } else {
            tempCursor.insertText(str);
            tempCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
            this->setTextCursor(tempCursor);
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
