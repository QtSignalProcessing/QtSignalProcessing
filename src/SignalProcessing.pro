#-------------------------------------------------
#
# Project created by QtCreator 2013-01-29T16:08:41
#
#-------------------------------------------------

QT += core widgets openglwidgets multimedia gui opengl

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VERSION_MAJOR = $$member(QT_VERSION, 0)
QT_VERSION_MINOR = $$member(QT_VERSION, 1)

isEqual(QT_VERSION_MAJOR, 4): QT += gui phonon
isEqual(QT_VERSION_MAJOR, 5): QT += widgets multimediawidgets multimedia

TARGET = SignalProcessing
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    plotwidget.cpp \
    iir.cpp \
    plotfilter.cpp \
    utilities.cpp \
    audiohandle.cpp \
    chebyshevfilter.cpp \
    glwidgetnew.cpp \
    glspectrum.cpp \




HEADERS  += mainwindow.h \
    plotwidget.h \
    iir.h \
    plotfilter.h \
    utilities.h \
    audiohandle.h \
    chebyshevfilter.h \
    glwidgetnew.h \
    glspectrum.h \




win32: LIBS += -L$$PWD/../../../../../../../msys64/mingw64/lib/ -lfftw3 -lsamplerate -lfreeglut -lglew32 -lopengl32

INCLUDEPATH += $$PWD/../../../../../../../msys64/mingw64/include
DEPENDPATH += $$PWD/../../../../../../../msys64/mingw64/include

win32: LIBS += -L$$PWD/../../libsndfile/lib/ -llibsndfile-1

INCLUDEPATH += $$PWD/../../libsndfile/include
DEPENDPATH += $$PWD/../../libsndfile/include
