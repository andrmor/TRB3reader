#ifndef AHIGHLIGHTERS_H
#define AHIGHLIGHTERS_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class AHighlighterScriptWindow : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    AHighlighterScriptWindow(QTextDocument *parent = 0);

    void setCustomCommands(QStringList functions, QStringList constants = QStringList());

protected:
    void highlightBlock(const QString &text);

//private:
    struct HighlightingRule
      {
        QRegularExpression pattern;
        QTextCharFormat format;
      };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat customKeywordFormat;
//    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat charFormat;
//    QTextCharFormat functionFormat;
};

class AHighlighterLrfScript : public AHighlighterScriptWindow
{
    Q_OBJECT
public:
    AHighlighterLrfScript(QTextDocument *parent = 0);

private:
    void setFixedVariables();
};

#endif // AHIGHLIGHTERS_H
