#-------------------------------------------------
#
# Project created by QtCreator 2018-07-30T13:19:00
#
#-------------------------------------------------

QT       += core gui sql widgets

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
        src/main.cpp \
        src/mainwindow.cpp \
        src/plaintextedit.cpp \
        src/highlighter.cpp \
        src/database.cpp \
        src/encryption.cpp \
        src/mdLite/tree.cpp \
        src/mdLite/tokenizer.cpp

HEADERS += \
        src/headers/mainwindow.h \
        src/headers/plaintextedit.h \
        src/headers/highlighter.h \
        src/headers/database.h \
        src/headers/encryption.h \
        src/mdLite/token.h \
        src/mdLite/tree.h \
        src/mdLite/tokenizer.h

FORMS += \
        mainwindow.ui

macx {
    QMAKE_CXXFLAGS += -std=c++11
    _BOOST_PATH = /usr/local/Cellar/boost/1.66.0
    INCLUDEPATH += "$${_BOOST_PATH}/include/"
    LIBS += -L$${_BOOST_PATH}/lib
    LIBS += -lboost_system -lboost_filesystem -lboost_regex
    LIBS += -framework AppKit -framework Foundation
    OBJECTIVE_SOURCES = src/macosuihandler.mm
    HEADERS += \
        src/headers/macosuihandler.h
    BUNDLE = $$OUT_PWD/$$TARGET$$quote(.app)/Contents
    QMAKE_POST_LINK += ditto \"$$PWD/html/header.html\" \"$$BUNDLE/Resources/\";
    QMAKE_POST_LINK += ditto \"$$PWD/html/footer.html\" \"$$BUNDLE/Resources/\";

    INCLUDEPATH += "/Users/dhamith/Downloads/cryptopp700/"
    LIBS += "/Users/dhamith/Downloads/cryptopp700/libcryptopp.a"
}

win32 {
    INCLUDEPATH += D:\windows\boost_1_67_0
    LIBS += -LD:\windows\boost_1_67_0\stage\lib \
            -llibboost_regex-mgw53-mt-d-x32-1_67 \
            -llibboost_filesystem-mgw53-mt-d-x32-1_67 \
            -llibboost_system-mgw53-mt-d-x32-1_67
}
