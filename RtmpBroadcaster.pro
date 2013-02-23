#-------------------------------------------------
#
# Project created by QtCreator 2013-02-13T15:38:23
#
#-------------------------------------------------

QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RtmpBroadcaster
TEMPLATE = app


SOURCES += main.cpp\
    screencap.cpp \
    encode.cpp \
    audiocap.cpp \
    audiocapdevice.cpp \
    camcap.cpp \
    videocompositor.cpp \
    audiocompositor.cpp \
    videocap.cpp \
    flvtag.cpp \
    rtmp.cpp

HEADERS  += \
    main.h \
    screencap.h \
    encode.h \
    audiocap.h \
    audiocapdevice.h \
    flvtag.h \
    camcap.h \
    videocompositor.h \
    audiocompositor.h \
    videocap.h \
    rtmp.h

FORMS    += settings.ui \
    mainwindow.ui

RESOURCES += \
    resource.qrc


LIBS += -lswscale -lx264 -lfdk-aac -lrtmp -lopencv_highgui -lopencv_core

OTHER_FILES +=

