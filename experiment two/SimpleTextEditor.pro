QT += widgets
CONFIG += c++17
TEMPLATE = app
TARGET = SimpleTextEditor

SOURCES += \
    main.cpp \
    codeeditor.cpp \
    mainwindow.cpp \
    finddialog.cpp \
    replacedialog.cpp \
    aboutdialog.cpp

HEADERS += \
    appinfo.h \
    codeeditor.h \
    mainwindow.h \
    finddialog.h \
    replacedialog.h \
    aboutdialog.h

FORMS += \
    mainwindow.ui \
    finddialog.ui \
    replacedialog.ui \
    aboutdialog.ui

win32-msvc*: QMAKE_CXXFLAGS += /utf-8
