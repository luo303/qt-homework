#ifndef WIDGET_H
#define WIDGET_H

#include <QMainWindow>

class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QMainWindow
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_actSetStudents_triggered();
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn,
                                           int previousRow, int previousColumn);
    void on_btnSetHeader_clicked();
    void on_btnSetRows_clicked();
    void on_btnInitialize_clicked();
    void on_btnInsertRow_clicked();
    void on_btnAddRow_clicked();
    void on_btnDeleteRow_clicked();
    void on_btnAutoRowHeight_clicked();
    void on_btnAutoColumnWidth_clicked();
    void on_btnReadTable_clicked();
    void on_chkEditable_toggled(bool checked);
    void on_chkAlternating_toggled(bool checked);
    void on_chkShowRowHeader_toggled(bool checked);
    void on_chkShowColumnHeader_toggled(bool checked);
    void on_radioSelectRow_toggled(bool checked);

private:
    void setInitialHeaders();
    void styleHeaderItems();
    void showOriginForRow(int row);

    Ui::Widget *ui;
    QLabel *m_originLabel;
};

#endif // WIDGET_H
