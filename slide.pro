#-------------------------------------------------
#
# Project created by QtCreator 2016-07-14T12:59:28
#
#-------------------------------------------------

QT       += core gui
QT       += xml
QT       += svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = slide
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    slide.cpp

HEADERS  += mainwindow.h \
    slide.h \
    catalog.h

FORMS    += mainwindow.ui \


win32:RC_FILE = ic.rc

RESOURCES += \
    sources.qrc
