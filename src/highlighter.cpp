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

    bracketBetweenFormat.setForeground(QColor("#aaaaff"));
    rule.pattern = QRegularExpression("\\[(.*?)\\]");
    rule.format = bracketBetweenFormat;
    highlightingRules.append(rule);

    paranBetweenFormat.setForeground(QColor("#eb41a4"));
    rule.pattern = QRegularExpression("\\((.*?)\\)");
    rule.format = paranBetweenFormat;
    highlightingRules.append(rule);

    tagFormat.setForeground(QColor("#6e4eff"));
    rule.pattern = QRegularExpression("#{1}\\w+");
    rule.format = tagFormat;
    highlightingRules.append(rule);

    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(QColor("#437bce"));
    QStringList keywordPatterns;
    keywordPatterns
            << "^(```)" << "^\\s*\\*\\s" << "^\\s*\\-\\s"  << "^(\\s*>){1,}"
            << "^(\\s*\\d+\\.\\s)" << "\\*" << "`" << "\\~\\~" << "\\(" << "\\)"
            << "\\[" << "\\]";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    inlineCodeFormat.setFontItalic(true);
    inlineCodeFormat.setForeground(QColor("#999999"));
    rule.pattern = QRegularExpression("(`(.*?[^`])`)");
    rule.format = inlineCodeFormat;
    highlightingRules.append(rule);


    h1Format.setFontPointSize(32);
    h1Format.setForeground(QColor("#0066ff"));
    rule.pattern = QRegularExpression("^#{1}\\s.+");
    rule.format = h1Format;
    highlightingRules.append(rule);

    h2Format.setFontPointSize(28);
    h2Format.setForeground(QColor("#0066ff"));
    rule.pattern = QRegularExpression("^#{2}\\s.+");
    rule.format = h2Format;
    highlightingRules.append(rule);

    h3Format.setFontPointSize(24);
    h3Format.setForeground(QColor("#0066ff"));
    rule.pattern = QRegularExpression("^#{3}\\s.+");
    rule.format = h3Format;
    highlightingRules.append(rule);

    h4Format.setFontPointSize(21);
    h4Format.setForeground(QColor("#0066ff"));
    rule.pattern = QRegularExpression("^#{4}\\s.+");
    rule.format = h4Format;
    highlightingRules.append(rule);

    h5Format.setFontPointSize(19);
    h5Format.setForeground(QColor("#0066ff"));
    rule.pattern = QRegularExpression("^#{5}\\s.+");
    rule.format = h5Format;
    highlightingRules.append(rule);

    h6Format.setFontPointSize(17);
    h6Format.setForeground(QColor("#0066ff"));
    rule.pattern = QRegularExpression("^#{6}\\s.+");
    rule.format = h6Format;
    highlightingRules.append(rule);
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

}
