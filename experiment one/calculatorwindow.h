#ifndef CALCULATORWINDOW_H
#define CALCULATORWINDOW_H

#include <QMainWindow>

class QKeyEvent;

QT_BEGIN_NAMESPACE
namespace Ui {
class CalculatorWindow;
}
QT_END_NAMESPACE

class CalculatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CalculatorWindow(QWidget *parent = nullptr);
    ~CalculatorWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void inputDecimalPoint();
    void calculateResult();
    void backspace();
    void clearEntry();
    void clearAll();
    void toggleSign();

private:
    void connectButtons();
    void resetAfterError();
    bool applyPendingOperation(double rightOperand);
    void showError(const QString &message);
    void enterDigit(int digit);
    void enterOperator(QChar operation);

    Ui::CalculatorWindow *ui;
    double storedValue = 0.0;
    QChar pendingOperator;
    bool waitingForOperand = true;
    bool justEvaluated = false;
    bool errorShown = false;
};

#endif // CALCULATORWINDOW_H
