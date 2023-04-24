QT       += core gui sql serialport texttospeech

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _CRT_SECURE_NO_WARNINGS

DESTDIR = $$PWD/bin
TARGET = FACE

SOURCES += \
    configwidget.cpp \
    face_sql.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \

HEADERS += \
    configwidget.h \
    face_sql.h \
    loginwidget.h \
    mainwindow.h
    submain.h

FORMS += \
    configwidget.ui \
    loginwidget.ui \
    mainwindow.ui

include($${PWD}/QSidePanel/q_side_panel.pri)

INCLUDEPATH += $$PWD/opencv/inc/
INCLUDEPATH += $$PWD/opencv/inc/opencv
INCLUDEPATH += $$PWD/opencv/inc/opencv2

INCLUDEPATH += $$PWD/arcface/inc
#LIBS += -L$$PWD/opencv/lib -lopencv_*
LIBS += -L$$PWD/opencv/lib -lopencv_core249     \
#                           -lopencv_core249d    \
                           -lopencv_highgui249  \
#                           -lopencv_highgui249d \
                           -lopencv_imgproc249  \
#                           -lopencv_imgproc249d \
                           -lopencv_objdetect249 \
                           -lopencv_video249



LIBS += -L$$PWD/arcface/lib -llibarcsoft_face_engine

RESOURCES += \
    resource.qrc

RC_ICONS = logo.ico







