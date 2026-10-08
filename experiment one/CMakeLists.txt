cmake_minimum_required(VERSION 3.16)

project(KeyboardCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets)

qt_add_executable(KeyboardCalculator
    main.cpp
    calculatorwindow.cpp
    calculatorwindow.h
    calculatorwindow.ui
)

target_link_libraries(KeyboardCalculator PRIVATE Qt6::Widgets)

if(MSVC)
    target_compile_options(KeyboardCalculator PRIVATE /utf-8)
endif()
