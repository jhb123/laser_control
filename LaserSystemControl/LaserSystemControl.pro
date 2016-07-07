#-------------------------------------------------
#
# Project created by QtCreator 2016-07-07T14:50:02
#
#-------------------------------------------------

QT       += core gui multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LaserSystemControl
TEMPLATE = app

include(Interbus/Interbus.pri)


SOURCES += \
    camera.cpp \
    edfa.cpp \
    laser.cpp \
    lasersystem.cpp \
    main.cpp \
    touchscreen.cpp

HEADERS  += \
    camera.h \
    edfa.h \
    laser.h \
    lasersystem.h \
    touchscreen.h

FORMS += \
    touchscreen.ui

RESOURCES += \
    buttonresources.qrc
