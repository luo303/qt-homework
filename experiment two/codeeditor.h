#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>

class CodeEditor;

// 行号栏：单独一个小控件贴在编辑区左侧，真正画行号的动作交给 CodeEditor，
// 这样它可以拿到 firstVisibleBlock() 等只有当父类是 QPlainTextEdit 时才有的信息。
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *m_editor;
};

// 参考 Qt 官方 Code Editor 例子，从 QPlainTextEdit 派生，加上行号显示和当前行高亮。
// 这样做的好处是原来的编辑功能全部保留，主窗口里只需要把控件换成这个子类。
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    // 查找：从当前光标位置往后（或往前）找，找不到时回到文档另一端再找一次，
    // wrapped 用来告诉调用方是否发生了“从头继续”的情况
    bool findNext(const QString &text, bool backward, bool caseSensitive,
                  bool *wrapped = nullptr);

public slots:
    void setLineNumberVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    LineNumberArea *m_lineNumberArea;
    bool m_lineNumberVisible;
};

#endif // CODEEDITOR_H
