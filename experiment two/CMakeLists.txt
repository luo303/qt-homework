cmake_minimum_required(VERSION 3.16)

project(SimpleTextEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)

qt_add_executable(SimpleTextEditor
    main.cpp
    appinfo.h
    codeeditor.cpp
    codeeditor.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    finddialog.cpp
    finddialog.h
    finddialog.ui
    replacedialog.cpp
    replacedialog.h
    replacedialog.ui
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
)

target_link_libraries(SimpleTextEditor PRIVATE Qt6::Widgets)

# uic 生成的 ui_mainwindow.h 里会 #include "codeeditor.h"（控件提升），
# 所以要把工程源码目录也加进头文件搜索路径
target_include_directories(SimpleTextEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(SimpleTextEditor PRIVATE /utf-8)
else()
    target_compile_options(SimpleTextEditor PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
