#-------------------------------------------------
#
# Project created by QtCreator 2014-11-01T15:42:25
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QT += serialport

QMAKE_CXXFLAGS += -std=c++11


TARGET = ADCqtplot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    settingsdialog.cpp \
    viewlog.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    settingsdialog.h \
    viewlog.h \
    util.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    viewlog.ui

OTHER_FILES +=

RESOURCES += \
    ADCqtplot.qrc
