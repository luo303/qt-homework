#include "calculatorwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Keyboard Calculator");

    CalculatorWindow window;
    window.show();
    return app.exec();
}
