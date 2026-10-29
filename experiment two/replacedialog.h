#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class CodeEditor;

QT_BEGIN_NAMESPACE
namespace Ui {
class ReplaceDialog;
}
QT_END_NAMESPACE

// 替换对话框：查找逻辑复用 CodeEditor::findNext()，这里只处理“替换”和“全部替换”
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(CodeEditor *editor, QWidget *parent = nullptr);
    ~ReplaceDialog() override;

    void setSearchText(const QString &text);

private slots:
    void findNext();
    void replaceOnce();
    void replaceAll();

private:
    bool findNextMatch(bool *wrapped);

    Ui::ReplaceDialog *ui;
    CodeEditor *m_editor;
};

#endif // REPLACEDIALOG_H
