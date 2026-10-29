#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "appinfo.h"

#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // 个人信息统一在 appinfo.h 里改，这里只负责显示
    ui->nameLabel->setText(AppInfo::appName());
    ui->infoLabel->setText(tr("开发者：%1\n学号：%2\nemail：%3")
                               .arg(AppInfo::developer(),
                                    AppInfo::studentId(),
                                    AppInfo::email()));
    ui->versionLabel->setText(AppInfo::version());

    connect(ui->okButton, &QPushButton::clicked, this, &QDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
