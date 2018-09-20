#include "headers/highlighter.h"
#include <QTextCharFormat>
#include <QDebug>

Highlighter::Highlighter(QObject *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    blockquoteFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("^(\\s?>){1,}.+");
    rule.format = blockquoteFormat;
    highlightingRules.append(rule);

    emFormat.setFontItalic(true);
    emFormat.setForeground(QColor("#ff6666"));
    emFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression("(\\*)(.*?)(\\*)");
    rule.format = emFormat;
    highlightingRules.append(rule);

    strongFormat.setFontWeight(QFont::Bold);
    strongFormat.setForeground(QColor("#fa7272"));
    rule.pattern = QRegularExpression("(\\*\\*)(.*?)(\\*\\*)");
    rule.format = strongFormat;
    highlightingRules.append(rule);

    strikeFormat.setFontStrikeOut(true);
    strikeFormat.setFontItalic(true);
    strikeFormat.setForeground(QColor("#c55f5f"));
    rule.pattern = QRegularExpression("(\\~\\~)(.*?)(\\~\\~)");
    rule.format = strikeFormat;
    highlightingRules.append(rule);

    tagFormat.setForeground(QColor("#6e4eff"));
    rule.pattern = QRegularExpression("#{1}\\w+");
    rule.format = tagFormat;
    highlightingRules.append(rule);

    inlineCodeFormat.setFontItalic(true);
    inlineCodeFormat.setForeground(QColor("#999999"));
    rule.pattern = QRegularExpression("(`(.*?[^`])`)");
    rule.format = inlineCodeFormat;
    highlightingRules.append(rule);

    h1Format.setFontPointSize(32);
    h1Format.setFontWeight(QFont::Bold);
    h1Format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    h1Format.setUnderlineColor(QColor("#A7A7A7"));
    h1Format.setForeground(QColor("#009BFF"));
    rule.pattern = QRegularExpression("^#{1}\\s.+");
    rule.format = h1Format;
    highlightingRules.append(rule);

    h2Format.setFontPointSize(28);
    h2Format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    h2Format.setUnderlineColor(QColor("#A7A7A7"));
    h2Format.setForeground(QColor("#009BFF"));
    rule.pattern = QRegularExpression("^#{2}\\s.+");
    rule.format = h2Format;
    highlightingRules.append(rule);

    h3Format.setFontPointSize(24);
    h3Format.setForeground(QColor("#009BFF"));
    rule.pattern = QRegularExpression("^#{3}\\s.+");
    rule.format = h3Format;
    highlightingRules.append(rule);

    h4Format.setFontPointSize(22);
    h4Format.setForeground(QColor("#009BFF"));
    rule.pattern = QRegularExpression("^#{4}\\s.+");
    rule.format = h4Format;
    highlightingRules.append(rule);

    h5Format.setFontPointSize(18);
    h5Format.setForeground(QColor("#009BFF"));
    rule.pattern = QRegularExpression("^#{5}\\s.+");
    rule.format = h5Format;
    highlightingRules.append(rule);

    h6Format.setFontPointSize(16);
    h6Format.setFontWeight(QFont::Bold);
    h6Format.setForeground(QColor("#009BFF"));
    rule.pattern = QRegularExpression("^#{6}\\s.+");
    rule.format = h6Format;
    highlightingRules.append(rule);

    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setForeground(QColor("#C6C6C6"));
    rule.pattern = QRegularExpression("^#{1,6}\\s");
    rule.format = headerFormat;
    highlightingRules.append(rule);

    codeBlockFormat.setForeground(QColor("#FF6B33"));
    codeStartExpression = QRegExp("^```(\\w|\\d|.[^\\s]){0,}\\^$");
    codeEndExpression = QRegExp("^```$");

    commentBlockFormat.setForeground(QColor("#0DFFC8"));
    commentBlockFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("<!--(.){0,}-->");
    rule.format = commentBlockFormat;
    highlightingRules.append(rule);

    symbolFormat.setFontWeight(QFont::Bold);
    symbolFormat.setForeground(QColor("#437bce"));
    QStringList symbolPatterns;
    symbolPatterns
            << "^(```)" << "^\\s*\\*\\s" << "^\\s*\\-\\s"  << "^(\\s*>){1,}"
            << "^(\\s*\\d+\\.\\s)" << "\\*" << "`" << "\\~\\~";
    foreach (const QString &pattern, symbolPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = symbolFormat;
        highlightingRules.append(rule);
    }

}

void Highlighter::highlightBlock(const QString &text) {

    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = codeStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = codeEndExpression.indexIn(text, startIndex);
        int codeLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            codeLength = text.length() - startIndex;
        } else {
            codeLength = endIndex - startIndex
                            + codeEndExpression.matchedLength();
        }
        setFormat(startIndex, codeLength, codeBlockFormat);
        startIndex = codeStartExpression.indexIn(text, startIndex + codeLength);
    }
}
