#-------------------------------------------------
#
# Project created by QtCreator 2018-03-21T08:11:42
#
#-------------------------------------------------

QT       += core gui widgets webenginewidgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickNote
TEMPLATE = app
win32:RC_ICONS += icons/QuickNote_icon.ico
ICON = icons/QuickNote_icon.icns

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    stringparser.cpp \
    htmlelement.cpp \
    newnotewindow.cpp \
    database.cpp \
    newwebenginenotewindow.cpp

HEADERS += \
        mainwindow.h \
    newnotewindow.h \
    newwebenginenotewindow.h

FORMS += \
        mainwindow.ui \
    newnotewindow.ui \
    newwebenginenotewindow.ui
