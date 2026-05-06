#include "codeeditor.h"
#include "linenumberarea.h"
#include "ofhighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QFont>
#include <QRegularExpression>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);
    m_highlighter = new OFHighlighter(document());

    QFont font("Consolas", 11);
    font.setStyleHint(QFont::Monospace);
    setFont(font);
    setTabStopDistance(40);  // 4 spaces width

    setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void CodeEditor::setLanguage(FileLanguage lang)
{
    if (m_language == lang) return;
    m_language = lang;
    m_highlighter->setLanguage(lang);
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extras;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(185);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extras.append(selection);
    }

    setExtraSelections(extras);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#F0F0F0"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#999999"));
            painter.drawText(0, top, m_lineNumberArea->width() - 4,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool CodeEditor::maybeSave()
{
    if (!document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, "OpenFOAM GUI",
                             "The file has been modified.\n"
                             "Do you want to save your changes?",
                             QMessageBox::Save | QMessageBox::Discard
                             | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        if (m_filePath.isEmpty())
            return false;
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void CodeEditor::toggleComment()
{
    // Determine comment prefix based on language
    QString commentPrefix;
    switch (m_language) {
    case FileLanguage::OpenFOAM:
    case FileLanguage::Cpp:
    case FileLanguage::CppHeader:
    case FileLanguage::C:
    case FileLanguage::JSON:
    case FileLanguage::CMake:
    case FileLanguage::XML:
        commentPrefix = "//";
        break;
    case FileLanguage::Bash:
    case FileLanguage::Python:
        commentPrefix = "#";
        break;
    default:
        commentPrefix = "//";
        break;
    }

    QTextCursor cursor = textCursor();
    int startPos, endPos;

    if (cursor.hasSelection()) {
        startPos = cursor.selectionStart();
        endPos   = cursor.selectionEnd();
    } else {
        // No selection → operate on current line
        cursor.movePosition(QTextCursor::StartOfBlock);
        startPos = cursor.position();
        cursor.movePosition(QTextCursor::EndOfBlock);
        endPos = cursor.position();
    }

    // Adjust start to beginning of its line
    QTextCursor startCur(document());
    startCur.setPosition(startPos);
    startCur.movePosition(QTextCursor::StartOfBlock);
    startPos = startCur.position();

    // Adjust end to end of its line (or keep exact if mid-line)
    QTextCursor endCur(document());
    endCur.setPosition(endPos);

    // Check if ALL non-empty lines in selection are already commented
    QTextCursor checkCur(document());
    checkCur.setPosition(startPos);
    bool allCommented = true;

    while (checkCur.position() < endPos) {
        checkCur.movePosition(QTextCursor::StartOfBlock);
        QString line = checkCur.block().text();
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty() && !trimmed.startsWith(commentPrefix)) {
            allCommented = false;
            break;
        }
        checkCur.movePosition(QTextCursor::NextBlock);
        if (checkCur.position() == checkCur.block().position()
            && !checkCur.movePosition(QTextCursor::NextBlock))
            break;
        if (checkCur.position() >= endPos) break;
    }

    // Operate line by line
    QTextCursor opCur(document());
    opCur.setPosition(startPos);
    opCur.beginEditBlock();

    while (opCur.position() < endPos || opCur.block().position() == startPos) {
        opCur.movePosition(QTextCursor::StartOfBlock);
        int blockStart = opCur.position();
        QString line = opCur.block().text();
        QString trimmed = line.trimmed();

        if (trimmed.isEmpty()) {
            // skip empty lines
            if (!opCur.movePosition(QTextCursor::NextBlock)) break;
            if (opCur.position() >= endPos && opCur.block().position() != startPos) break;
            continue;
        }

        // Find leading whitespace
        int leadingSpaces = 0;
        while (leadingSpaces < line.length() && line[leadingSpaces].isSpace())
            leadingSpaces++;

        if (allCommented) {
            // Remove comment prefix
            int commentStart = line.indexOf(commentPrefix, leadingSpaces);
            if (commentStart >= 0) {
                opCur.setPosition(blockStart + commentStart);
                opCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                   commentPrefix.length());
                opCur.removeSelectedText();

                // Remove trailing space after comment prefix
                opCur.setPosition(blockStart + commentStart);
                QTextCursor peekCur(document());
                peekCur.setPosition(blockStart + commentStart);
                peekCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
                if (peekCur.selectedText() == " ") {
                    peekCur.removeSelectedText();
                }
            }
        } else {
            // Add comment prefix after leading whitespace
            opCur.setPosition(blockStart + leadingSpaces);
            opCur.insertText(commentPrefix + " ");
        }

        if (!opCur.movePosition(QTextCursor::NextBlock)) break;
        if (opCur.position() >= endPos && opCur.block().position() != startPos) break;
    }

    opCur.endEditBlock();

    // Restore a reasonable selection
    QTextCursor restoreCur(document());
    restoreCur.setPosition(startPos);
    setTextCursor(restoreCur);
}
