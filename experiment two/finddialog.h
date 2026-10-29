#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class CodeEditor;

QT_BEGIN_NAMESPACE
namespace Ui {
class FindDialog;
}
QT_END_NAMESPACE

// 查找对话框：只负责收集查找条件和给出提示，真正的查找在 CodeEditor::findNext() 里
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(CodeEditor *editor, QWidget *parent = nullptr);
    ~FindDialog() override;

    void setSearchText(const QString &text);

private slots:
    void findNext();

private:
    Ui::FindDialog *ui;
    CodeEditor *m_editor;
};

#endif // FINDDIALOG_H
