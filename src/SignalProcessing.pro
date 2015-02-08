#-------------------------------------------------
#
# Project created by QtCreator 2013-01-29T16:08:41
#
#-------------------------------------------------

QT += core opengl

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



win32: INCLUDEPATH  += ../include

unix: LIBS += -lsndfile -lfftw3 -lsamplerate
win32: LIBS += ../lib/win32/sndfile.lib ../lib/win32/fftw3.lib ../lib/win32/samplerate.lib
