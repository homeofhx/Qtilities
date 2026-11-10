QT += core multimedia testlib
QT -= gui

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TARGET = core_tests
TEMPLATE = app

INCLUDEPATH += ..

SOURCES += \
    core_tests.cpp \
    ../countdown/countdowntimer.cpp \
    ../RTC/chimerenderer.cpp

HEADERS += \
    ../countdown/countdowntimer.h \
    ../RTC/chimerenderer.h
