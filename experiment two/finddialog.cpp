#include "finddialog.h"
#include "ui_finddialog.h"

#include "codeeditor.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

FindDialog::FindDialog(CodeEditor *editor, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
    , m_editor(editor)
{
    ui->setupUi(this);

    // 非模态：查找窗口开着的时候还能直接点编辑区修改内容
    setModal(false);

    connect(ui->findButton, &QPushButton::clicked, this, &FindDialog::findNext);
    connect(ui->findEdit, &QLineEdit::returnPressed, this, &FindDialog::findNext);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 换了查找词就把上一次的提示清掉，免得留在界面上误导人
    connect(ui->findEdit, &QLineEdit::textEdited, ui->hintLabel, &QLabel::clear);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setSearchText(const QString &text)
{
    ui->findEdit->setText(text);
    ui->findEdit->selectAll();
}

void FindDialog::findNext()
{
    const QString text = ui->findEdit->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("查找"), tr("请先输入要查找的内容。"));
        return;
    }

    bool wrapped = false;
    const bool found = m_editor->findNext(text,
                                          ui->upRadio->isChecked(),
                                          ui->caseCheckBox->isChecked(),
                                          &wrapped);

    if (!found) {
        ui->hintLabel->clear();
        QMessageBox::information(this, tr("查找"), tr("未找到「%1」。").arg(text));
        return;
    }

    // 绕回文档另一端时只是提示一下，不弹窗，避免连续查找被打断
    ui->hintLabel->setText(wrapped ? tr("已到达文档另一端，已从头继续查找。")
                                   : QString());
}
