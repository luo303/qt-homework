QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TEMPLATE = app
TARGET = samp4_09Combobox

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    周四点名册.xls

# Ensure Chinese UTF-8 source text is compiled correctly with MSVC.
win32-msvc*: QMAKE_CXXFLAGS += /utf-8
