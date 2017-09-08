#-------------------------------------------------
#
# Project created by QtCreator 2015-08-24T14:24:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Imaging-with-Qt-and-CUDA
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    subimagesdialog.cpp \
    imagesiftf.cpp \
    siftdescriptor.cpp \

HEADERS  += mainwindow.h \
    subimagesdialog.h \
    siftdescriptor.h \
    imagesiftf.h

FORMS    += mainwindow.ui \
    subimagesdialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Imaging-functions-CUDA/x64/Release -lImaging-functions-CUDA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Imaging-functions-CUDA/x64/Release -lImaging-functions-CUDA

INCLUDEPATH += $$PWD/../Imaging-functions-CUDA/x64/Release \
            "D:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.0\include"
DEPENDPATH += $$PWD/../Imaging-functions-CUDA/x64/Release

RESOURCES += \
    icons.qrc
