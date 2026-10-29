#include "codeeditor.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_lineNumberVisible(true)
{
    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth();
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    // 行号栏宽度跟着总行数的位数变化，行数上万时也不会被截断
    int digits = 1;
    int maxLines = qMax(1, blockCount());
    while (maxLines >= 10) {
        maxLines /= 10;
        ++digits;
    }

    return 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);

    // 行号栏和当前行高亮都跟着调色板走，深色主题下不会突然出现一块亮白条
    const QColor base = palette().color(QPalette::Base);
    const bool dark = base.lightness() < 128;
    const QColor gutterColor = dark ? base.darker(118) : QColor(0xF2, 0xF2, 0xF2);

    painter.fillRect(event->rect(), gutterColor);

    QColor numberColor = palette().color(QPalette::Text);
    numberColor.setAlpha(150);
    painter.setPen(numberColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    // 只画当前可见区域里的行号，翻页、滚动都不用整篇重画
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.drawText(0, top,
                             m_lineNumberArea->width() - 4,
                             fontMetrics().height(),
                             Qt::AlignRight,
                             QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::setLineNumberVisible(bool visible)
{
    if (m_lineNumberVisible == visible)
        return;

    m_lineNumberVisible = visible;
    m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth();
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect rect = contentsRect();
    m_lineNumberArea->setGeometry(QRect(rect.left(), rect.top(),
                                        lineNumberAreaWidth(), rect.height()));
}

void CodeEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(m_lineNumberVisible ? lineNumberAreaWidth() : 0, 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!m_lineNumberVisible)
        return;

    if (dy != 0)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly()) {
        const QColor base = palette().color(QPalette::Base);
        const QColor lineColor = base.lightness() < 128 ? base.lighter(118)
                                                        : QColor(0xFF, 0xFC, 0xE6);

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);
}

void CodeEditor::changeEvent(QEvent *event)
{
    QPlainTextEdit::changeEvent(event);

    // 换了编辑器背景色以后，行号栏也要跟着重画一次
    if (event->type() == QEvent::PaletteChange) {
        if (m_lineNumberArea != nullptr)
            m_lineNumberArea->update();
        highlightCurrentLine();
    }
}

bool CodeEditor::findNext(const QString &text, bool backward, bool caseSensitive,
                          bool *wrapped)
{
    if (wrapped != nullptr)
        *wrapped = false;

    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor cursor = textCursor();
    const int from = backward ? cursor.selectionStart() : cursor.selectionEnd();

    QTextCursor found = document()->find(text, from, flags);

    // 到文档末尾（开头）还没找到，就回到另一端再找一遍，这样连续查找可以绕回来
    if (found.isNull()) {
        const int restart = backward ? document()->characterCount() : 0;
        found = document()->find(text, restart, flags);
        if (!found.isNull() && wrapped != nullptr)
            *wrapped = true;
    }

    if (found.isNull())
        return false;

    setTextCursor(found);
    ensureCursorVisible();
    return true;
}
