#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

#include "languagedetector.h"

class LineNumberArea;
class OFHighlighter;
class QCompleter;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth() const;

    void setFileName(const QString &path) { m_filePath = path; }
    QString fileName() const { return m_filePath; }

    void setLanguage(FileLanguage lang);
    FileLanguage language() const { return m_language; }

    void setReadOnly(bool ro) { QPlainTextEdit::setReadOnly(ro); }
    bool maybeSave();

    void toggleComment();
    void setAutoCompletion(bool enabled);

signals:
    void fileSaved(const QString &path);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void insertCompletion(const QString &text);

private:
    void setupCompleter();
    QString wordUnderCursor() const;

    QWidget *m_lineNumberArea;
    OFHighlighter *m_highlighter;
    QCompleter *m_completer = nullptr;
    QString m_filePath;
    FileLanguage m_language = FileLanguage::Unknown;
    bool m_autoComplete = true;
};

#endif // CODEEDITOR_H
