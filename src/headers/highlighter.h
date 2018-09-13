#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class Highlighter : public QSyntaxHighlighter
{
    public:
        Highlighter(QObject *parent);
        void highlightBlock(const QString &text);
    private:
        struct HighlightingRule
        {
            QRegularExpression pattern;
            QTextCharFormat format;
        };
        QVector<HighlightingRule> highlightingRules;

        QRegularExpression commentStartExpression;
        QRegularExpression commentEndExpression;

        QRegularExpression codeStartExpression;
        QRegularExpression codeEndExpression;

        QTextCharFormat keywordFormat;
        QTextCharFormat headerFormat;
        QTextCharFormat h1Format;
        QTextCharFormat h2Format;
        QTextCharFormat h3Format;
        QTextCharFormat h4Format;
        QTextCharFormat h5Format;
        QTextCharFormat h6Format;
        QTextCharFormat listFormat;
        QTextCharFormat blockquoteFormat;
        QTextCharFormat codeFormat;
        QTextCharFormat linkFormat;
        QTextCharFormat inlineCodeFormat;
        QTextCharFormat strongFormat;
        QTextCharFormat emFormat;
        QTextCharFormat strikeFormat;
        QTextCharFormat tagFormat;
};

#endif // HIGHLIGHTER_H
