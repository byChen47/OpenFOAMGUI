#ifndef OFHIGHLIGHTER_H
#define OFHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QMap>
#include <QVector>

#include "languagedetector.h"

class QTextDocument;

class OFHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit OFHighlighter(QTextDocument *parent = nullptr);

    void setLanguage(FileLanguage lang);
    FileLanguage language() const { return m_language; }

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    struct MultiLineRule {
        QRegularExpression startPattern;
        QRegularExpression endPattern;
        QTextCharFormat format;
    };

    // Per-language rule sets
    QVector<HighlightRule>  m_rules;
    QVector<MultiLineRule>  m_multiRules;
    FileLanguage            m_language = FileLanguage::Unknown;

    // ── Language setup helpers ──
    void setupOpenFOAM();
    void setupCpp();
    void setupBash();
    void setupPython();
    void setupJSON();
    void setupCMake();
    void setupPlainText();
    void clearRules();

    // ── Multi-line comment handling ──
    bool matchMultiLine(const QString &text, const MultiLineRule &mlRule);
};

#endif // OFHIGHLIGHTER_H
