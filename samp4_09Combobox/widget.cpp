#include "widget.h"
#include "ui_widget.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QFont>
#include <QHeaderView>
#include <QLabel>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidgetItem>

namespace {

struct Student
{
    const char *studentId;
    const char *name;
    const char *gender;
    const char *administrativeClass;
    const char *department;
    const char *major;
    const char *studyType;
    const char *origin;
};

// 学号、姓名和行政班级来自“周四点名册.xls”。
// 籍贯保存在姓名 item 的 Qt::UserRole 中，点击按钮后不显示，选中行时才显示。
const Student kStudents[] = {
    {"2024414290228", "吕泽凯",   "男", "2024软件2班", "网络空间安全学院", "软件工程", "初修", "广东省"},
    {"2024414290229", "罗宇丰",   "男", "2024软件2班", "网络空间安全学院", "软件工程", "初修", "广东省"},
    {"2024414290230", "罗棕辉",   "男", "2024软件2班", "网络空间安全学院", "软件工程", "初修", "江西省"},
    {"2024414290231", "罗琰",     "女", "2024软件2班", "网络空间安全学院", "软件工程", "初修", "广东省"},
    {"2024414290232", "欧阳宇绵", "女", "2024软件2班", "网络空间安全学院", "软件工程", "初修", "广东省"}
};

const int kStudentCount = sizeof(kStudents) / sizeof(kStudents[0]);
const int kNameColumn = 1;
const char kMyStudentId[] = "2024414290230";

QString fromUtf8(const char *text)
{
    return QString::fromUtf8(text);
}

} // namespace

Widget::Widget(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Widget),
      m_originLabel(new QLabel(this))
{
    ui->setupUi(this);

    m_originLabel->setMinimumWidth(240);
    m_originLabel->clear();
    ui->statusbar->addPermanentWidget(m_originLabel, 1);

    ui->tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(30);

    setInitialHeaders();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setInitialHeaders()
{
    const QStringList headers = {
        tr("姓名"), tr("性别"), tr("出生日期"),
        tr("民族"), tr("分数"), tr("是否党员")
    };

    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    styleHeaderItems();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_originLabel->clear();
}

void Widget::styleHeaderItems()
{
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        QTableWidgetItem *headerItem = ui->tableWidget->horizontalHeaderItem(column);
        if (!headerItem)
            continue;

        QFont font = headerItem->font();
        font.setBold(true);
        headerItem->setFont(font);
        headerItem->setForeground(QBrush(Qt::red));
        headerItem->setTextAlignment(Qt::AlignCenter);
    }
}

void Widget::on_actSetStudents_triggered()
{
    const QStringList headers = {
        tr("学号"), tr("姓名"), tr("性别"), tr("行政班级"),
        tr("院系"), tr("专业"), tr("修读性质")
    };

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setRowCount(kStudentCount);
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    for (int row = 0; row < kStudentCount; ++row) {
        const Student &student = kStudents[row];
        const QStringList values = {
            QString::fromLatin1(student.studentId),
            fromUtf8(student.name),
            fromUtf8(student.gender),
            fromUtf8(student.administrativeClass),
            fromUtf8(student.department),
            fromUtf8(student.major),
            fromUtf8(student.studyType)
        };

        for (int column = 0; column < values.size(); ++column) {
            QTableWidgetItem *item = new QTableWidgetItem(values.at(column));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row, column, item);
        }

        QTableWidgetItem *nameItem = ui->tableWidget->item(row, kNameColumn);
        nameItem->setData(Qt::UserRole, fromUtf8(student.origin));

        if (QString::fromLatin1(student.studentId) == QString::fromLatin1(kMyStudentId)) {
            for (int column = 0; column <= kNameColumn; ++column) {
                QTableWidgetItem *item = ui->tableWidget->item(row, column);
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
                item->setForeground(QBrush(Qt::red));
            }
        }
    }

    styleHeaderItems();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->clearSelection();
    ui->tableWidget->setCurrentCell(-1, -1);
    ui->tableWidget->blockSignals(false);

    ui->spinRowCount->setValue(kStudentCount);
    m_originLabel->clear();
}

void Widget::on_tableWidget_currentCellChanged(int currentRow, int currentColumn,
                                                int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    showOriginForRow(currentRow);
}

void Widget::showOriginForRow(int row)
{
    if (row < 0 || row >= ui->tableWidget->rowCount()) {
        m_originLabel->clear();
        return;
    }

    QTableWidgetItem *nameItem = ui->tableWidget->item(row, kNameColumn);
    if (!nameItem || !nameItem->data(Qt::UserRole).isValid()) {
        m_originLabel->clear();
        return;
    }

    m_originLabel->setText(tr("籍贯：%1").arg(nameItem->data(Qt::UserRole).toString()));
}

void Widget::on_btnSetHeader_clicked()
{
    setInitialHeaders();
}

void Widget::on_btnSetRows_clicked()
{
    ui->tableWidget->setRowCount(ui->spinRowCount->value());
}

void Widget::on_btnInitialize_clicked()
{
    on_actSetStudents_triggered();
}

void Widget::on_btnInsertRow_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        row = 0;
    ui->tableWidget->insertRow(row);
}

void Widget::on_btnAddRow_clicked()
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
}

void Widget::on_btnDeleteRow_clicked()
{
    const int row = ui->tableWidget->currentRow();
    if (row >= 0)
        ui->tableWidget->removeRow(row);
}

void Widget::on_btnAutoRowHeight_clicked()
{
    ui->tableWidget->resizeRowsToContents();
}

void Widget::on_btnAutoColumnWidth_clicked()
{
    ui->tableWidget->resizeColumnsToContents();
}

void Widget::on_btnReadTable_clicked()
{
    QStringList lines;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QStringList values;
        for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
            QTableWidgetItem *item = ui->tableWidget->item(row, column);
            values << (item ? item->text() : QString());
        }
        lines << values.join(QStringLiteral("\t"));
    }
    ui->plainTextEdit->setPlainText(lines.join(QStringLiteral("\n")));
}

void Widget::on_chkEditable_toggled(bool checked)
{
    ui->tableWidget->setEditTriggers(checked
        ? QAbstractItemView::AllEditTriggers
        : QAbstractItemView::NoEditTriggers);
}

void Widget::on_chkAlternating_toggled(bool checked)
{
    ui->tableWidget->setAlternatingRowColors(checked);
}

void Widget::on_chkShowRowHeader_toggled(bool checked)
{
    ui->tableWidget->verticalHeader()->setVisible(checked);
}

void Widget::on_chkShowColumnHeader_toggled(bool checked)
{
    ui->tableWidget->horizontalHeader()->setVisible(checked);
}

void Widget::on_radioSelectRow_toggled(bool checked)
{
    ui->tableWidget->setSelectionBehavior(checked
        ? QAbstractItemView::SelectRows
        : QAbstractItemView::SelectItems);
}
