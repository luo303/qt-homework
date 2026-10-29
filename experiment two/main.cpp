#include "mainwindow.h"

#include "appinfo.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QStyle>
#include <QStyleFactory>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(AppInfo::appName());
    QApplication::setApplicationVersion(AppInfo::version());

    // 加载 Qt 自带的简体中文翻译，让 QMessageBox、颜色/字体对话框也是中文
    QTranslator qtTranslator;
    const QString transPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    const bool loaded = qtTranslator.load(QLocale(), QStringLiteral("qtbase"),
                                          QStringLiteral("_"), transPath)
                        || qtTranslator.load(QLocale(), QStringLiteral("qt"),
                                             QStringLiteral("_"), transPath);
    if (loaded)
        app.installTranslator(&qtTranslator);

    // Windows 开着深色模式时，主窗口会变黑、而菜单弹出层还是白的，看着很不协调。
    // 这里统一换成 Fusion 风格 + 一套固定的浅色配色，跟实验指导书的界面截图保持一致。
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette light;
    light.setColor(QPalette::Window, QColor(0xF0, 0xF0, 0xF0));
    light.setColor(QPalette::WindowText, Qt::black);
    light.setColor(QPalette::Base, Qt::white);
    light.setColor(QPalette::AlternateBase, QColor(0xF7, 0xF7, 0xF7));
    light.setColor(QPalette::Text, Qt::black);
    light.setColor(QPalette::Button, QColor(0xF0, 0xF0, 0xF0));
    light.setColor(QPalette::ButtonText, Qt::black);
    light.setColor(QPalette::ToolTipBase, QColor(0xFF, 0xFF, 0xDC));
    light.setColor(QPalette::ToolTipText, Qt::black);
    light.setColor(QPalette::Highlight, QColor(0x30, 0x8C, 0xCC));
    light.setColor(QPalette::HighlightedText, Qt::white);
    QApplication::setPalette(light);

    MainWindow window;
    window.show();

    return app.exec();
}
