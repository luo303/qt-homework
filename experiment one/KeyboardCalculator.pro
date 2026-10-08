QT += widgets
CONFIG += c++17
TEMPLATE = app
TARGET = KeyboardCalculator

SOURCES += \
    main.cpp \
    calculatorwindow.cpp

HEADERS += calculatorwindow.h
FORMS += calculatorwindow.ui

win32-msvc*: QMAKE_CXXFLAGS += /utf-8
