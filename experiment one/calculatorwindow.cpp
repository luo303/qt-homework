#include "calculatorwindow.h"
#include "ui_calculatorwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <cmath>

CalculatorWindow::CalculatorWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::CalculatorWindow)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    connectButtons();
}

CalculatorWindow::~CalculatorWindow()
{
    delete ui;
}

void CalculatorWindow::connectButtons()
{
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons)
        button->setFocusPolicy(Qt::NoFocus);

    const QList<QPushButton *> digitButtons = {
        ui->digit0Button, ui->digit1Button, ui->digit2Button, ui->digit3Button,
        ui->digit4Button, ui->digit5Button, ui->digit6Button, ui->digit7Button,
        ui->digit8Button, ui->digit9Button
    };
    for (QPushButton *button : digitButtons) {
        connect(button, &QPushButton::clicked, this, [this, button] {
            enterDigit(button->text().toInt());
        });
    }

    connect(ui->decimalButton, &QPushButton::clicked, this, &CalculatorWindow::inputDecimalPoint);
    connect(ui->clearEntryButton, &QPushButton::clicked, this, &CalculatorWindow::clearEntry);
    connect(ui->clearButton, &QPushButton::clicked, this, &CalculatorWindow::clearAll);
    connect(ui->backspaceButton, &QPushButton::clicked, this, &CalculatorWindow::backspace);
    connect(ui->signButton, &QPushButton::clicked, this, &CalculatorWindow::toggleSign);
    connect(ui->equalsButton, &QPushButton::clicked, this, &CalculatorWindow::calculateResult);

    connect(ui->addButton, &QPushButton::clicked, this, [this] { enterOperator('+'); });
    connect(ui->subtractButton, &QPushButton::clicked, this, [this] { enterOperator('-'); });
    connect(ui->multiplyButton, &QPushButton::clicked, this, [this] { enterOperator('*'); });
    connect(ui->divideButton, &QPushButton::clicked, this, [this] { enterOperator('/'); });
}

void CalculatorWindow::enterDigit(int digit)
{
    if (errorShown)
        resetAfterError();

    if (waitingForOperand || justEvaluated) {
        ui->displayLabel->setText(QString::number(digit));
        waitingForOperand = false;
        justEvaluated = false;
    } else if (ui->displayLabel->text() == "0" || ui->displayLabel->text() == "-0") {
        const bool negative = ui->displayLabel->text().startsWith('-');
        ui->displayLabel->setText((negative ? QStringLiteral("-") : QString())
                                  + QString::number(digit));
    } else {
        ui->displayLabel->setText(ui->displayLabel->text() + QString::number(digit));
    }
}

void CalculatorWindow::inputDecimalPoint()
{
    if (errorShown)
        resetAfterError();

    if (waitingForOperand || justEvaluated) {
        ui->displayLabel->setText("0.");
        waitingForOperand = false;
        justEvaluated = false;
    } else if (!ui->displayLabel->text().contains('.')) {
        ui->displayLabel->setText(ui->displayLabel->text() + '.');
    }
}

void CalculatorWindow::enterOperator(QChar operation)
{
    if (errorShown)
        return;

    bool ok = false;
    const double currentValue = ui->displayLabel->text().toDouble(&ok);
    if (!ok)
        return;

    // A new operator completes the preceding operation. Multiplication and
    // division stay in the current term; addition and subtraction finish it.
    if (pendingOperator.isNull()) {
        storedValue = 0.0;
        currentTerm = currentValue;
        currentTermSign = 1;
    } else if (!waitingForOperand) {
        if (!applyPendingOperation(currentValue))
            return;
    }

    pendingOperator = operation;
    waitingForOperand = true;
    justEvaluated = false;

    const double preview = storedValue + currentTermSign * currentTerm;
    if (std::isfinite(preview))
        ui->displayLabel->setText(QString::number(preview, 'g', 15));
}

bool CalculatorWindow::applyPendingOperation(double rightOperand)
{
    switch (pendingOperator.toLatin1()) {
    case '+':
    case '-':
        storedValue += currentTermSign * currentTerm;
        currentTerm = rightOperand;
        currentTermSign = pendingOperator == QLatin1Char('+') ? 1 : -1;
        break;
    case '*': currentTerm *= rightOperand; break;
    case '/':
        if (rightOperand == 0.0) {
            showError("除数不能为 0");
            return false;
        }
        currentTerm /= rightOperand;
        break;
    default:
        return false;
    }

    if (!std::isfinite(storedValue) || !std::isfinite(currentTerm)) {
        showError("结果超出范围");
        return false;
    }
    return true;
}

void CalculatorWindow::calculateResult()
{
    if (errorShown || pendingOperator.isNull())
        return;

    bool ok = false;
    const double rightOperand = ui->displayLabel->text().toDouble(&ok);
    if (!ok || !applyPendingOperation(rightOperand))
        return;

    const double result = storedValue + currentTermSign * currentTerm;
    if (!std::isfinite(result)) {
        showError("结果超出范围");
        return;
    }

    // 15 significant digits keep the result readable without the common
    // floating point tail from decimal calculations.
    ui->displayLabel->setText(QString::number(result, 'g', 15));

    storedValue = 0.0;
    currentTerm = 0.0;
    currentTermSign = 1;
    pendingOperator = QChar();
    waitingForOperand = true;
    justEvaluated = true;
}

void CalculatorWindow::backspace()
{
    if (errorShown) {
        resetAfterError();
        return;
    }
    if (waitingForOperand && !justEvaluated)
        return;

    QString entry = ui->displayLabel->text();
    entry.chop(1);
    if (entry.isEmpty() || entry == "-")
        entry = "0";
    ui->displayLabel->setText(entry);
    waitingForOperand = false;
    justEvaluated = false;
}

void CalculatorWindow::clearEntry()
{
    ui->displayLabel->setText("0");
    waitingForOperand = true;
    justEvaluated = false;
    errorShown = false;
}

void CalculatorWindow::clearAll()
{
    storedValue = 0.0;
    currentTerm = 0.0;
    currentTermSign = 1;
    pendingOperator = QChar();
    waitingForOperand = true;
    justEvaluated = false;
    errorShown = false;
    ui->displayLabel->setText("0");
}

void CalculatorWindow::toggleSign()
{
    if (errorShown)
        resetAfterError();

    QString entry = ui->displayLabel->text();
    if (entry == "0") {
        entry = "-0";
    } else if (entry.startsWith('-')) {
        entry.remove(0, 1);
    } else {
        entry.prepend('-');
    }
    ui->displayLabel->setText(entry);
    waitingForOperand = false;
    justEvaluated = false;
}

void CalculatorWindow::showError(const QString &message)
{
    ui->displayLabel->setText(message);
    pendingOperator = QChar();
    waitingForOperand = true;
    justEvaluated = false;
    errorShown = true;
}

void CalculatorWindow::resetAfterError()
{
    clearAll();
}

void CalculatorWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    if (event->text() == "*") {
        enterOperator('*');
    } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        enterDigit(key - Qt::Key_0);
    } else if (key == Qt::Key_Period || key == Qt::Key_Comma) {
        inputDecimalPoint();
    } else if (key == Qt::Key_Plus) {
        enterOperator('+');
    } else if (key == Qt::Key_Minus) {
        enterOperator('-');
    } else if (key == Qt::Key_Asterisk) {
        enterOperator('*');
    } else if (key == Qt::Key_Slash) {
        enterOperator('/');
    } else if (key == Qt::Key_Equal || key == Qt::Key_Return || key == Qt::Key_Enter) {
        calculateResult();
    } else if (key == Qt::Key_Backspace) {
        backspace();
    } else if (key == Qt::Key_Delete) {
        clearEntry();
    } else if (key == Qt::Key_Escape) {
        clearAll();
    } else {
        QMainWindow::keyPressEvent(event);
        return;
    }
    event->accept();
}
