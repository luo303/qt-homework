#include "replacedialog.h"
#include "ui_replacedialog.h"

#include "codeeditor.h"

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>

ReplaceDialog::ReplaceDialog(CodeEditor *editor, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
    , m_editor(editor)
{
    ui->setupUi(this);

    // 和查找对话框一样用非模态，替换的时候还能看到编辑区里的效果
    setModal(false);

    connect(ui->findButton, &QPushButton::clicked, this, &ReplaceDialog::findNext);
    connect(ui->replaceButton, &QPushButton::clicked, this, &ReplaceDialog::replaceOnce);
    connect(ui->replaceAllButton, &QPushButton::clicked, this, &ReplaceDialog::replaceAll);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(ui->findEdit, &QLineEdit::textEdited, ui->hintLabel, &QLabel::clear);
    connect(ui->replaceEdit, &QLineEdit::textEdited, ui->hintLabel, &QLabel::clear);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setSearchText(const QString &text)
{
    ui->findEdit->setText(text);
    ui->findEdit->selectAll();
}

bool ReplaceDialog::findNextMatch(bool *wrapped)
{
    const QString target = ui->findEdit->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请先输入要查找的内容。"));
        return false;
    }

    bool w = false;
    const bool found = m_editor->findNext(target,
                                          ui->upRadio->isChecked(),
                                          ui->caseCheckBox->isChecked(),
                                          &w);
    if (wrapped != nullptr)
        *wrapped = w;

    if (!found) {
        ui->hintLabel->clear();
        QMessageBox::information(this, tr("替换"), tr("未找到「%1」。").arg(target));
    }

    return found;
}

void ReplaceDialog::findNext()
{
    bool wrapped = false;
    if (!findNextMatch(&wrapped))
        return;

    ui->hintLabel->setText(wrapped ? tr("已到达文档另一端，已从头继续查找。")
                                   : QString());
}

void ReplaceDialog::replaceOnce()
{
    const QString target = ui->findEdit->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请先输入要查找的内容。"));
        return;
    }

    const Qt::CaseSensitivity sensitivity = ui->caseCheckBox->isChecked()
                                                ? Qt::CaseSensitive
                                                : Qt::CaseInsensitive;

    QTextCursor cursor = m_editor->textCursor();

    // 只有当当前选中的内容确实是查找词时才替换，否则先找到它，
    // 免得选中了别的文字按“替换”把无关内容改掉
    if (cursor.hasSelection()
        && QString::compare(cursor.selectedText(), target, sensitivity) == 0) {
        cursor.insertText(ui->replaceEdit->text());
        m_editor->setTextCursor(cursor);
    }

    findNext();     // 替换完（或没替换）都顺手跳到下一个
}

void ReplaceDialog::replaceAll()
{
    const QString target = ui->findEdit->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请先输入要查找的内容。"));
        return;
    }

    QTextDocument::FindFlags flags;
    if (ui->caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QTextDocument *document = m_editor->document();
    QTextCursor cursor(document);
    int count = 0;

    cursor.beginEditBlock();    // 合成一次编辑，撤销时一步就能退回
    while (true) {
        QTextCursor found = document->find(target, cursor, flags);
        if (found.isNull())
            break;

        found.insertText(ui->replaceEdit->text());
        cursor = found;
        ++count;
    }
    cursor.endEditBlock();

    if (count == 0) {
        ui->hintLabel->clear();
        QMessageBox::information(this, tr("替换"), tr("未找到「%1」。").arg(target));
        return;
    }

    ui->hintLabel->setText(tr("共替换 %1 处。").arg(count));
}
