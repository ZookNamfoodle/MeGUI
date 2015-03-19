#-------------------------------------------------
#
# Project created by QtCreator 2014-12-11T00:36:51
#
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HPC
TEMPLATE = app

INCLUDEPATH += /opt/local/include
INCLUDEPATH += /opt/local/include/opencv2
INCLUDEPATH += /opt/local/include/opencv

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

LIBS += -L/opt/local/lib
LIBS += -lopencv_calib3d \
-lopencv_contrib \
-lopencv_core \
-lopencv_features2d \
-lopencv_flann \
-lopencv_gpu \
-lopencv_highgui \
-lopencv_imgproc \
-lopencv_legacy \
-lopencv_ml \
-lopencv_objdetect \
-lopencv_video\
-lopencv_nonfree

LIBS += -lopencv_calib3d \
-lopencv_contrib \
-lopencv_core \
-lopencv_features2d \
-lopencv_flann \
-lopencv_gpu \
-lopencv_highgui \
-lopencv_imgproc \
-lopencv_legacy \
-lopencv_ml \
-lopencv_objdetect \
-lopencv_video


SOURCES += main.cpp \
    HPC.cpp


HEADERS  += \
    HPC.h

FORMS    += \
    HPC.ui
