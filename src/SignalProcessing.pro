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
isEqual(QT_VERSION_MAJOR, 5): QT += widgets multimediawidgets

TARGET = SignalProcessing
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    plotwidget.cpp \
    glospectrum.cpp \
    iir.cpp \
    plotfilter.cpp \
    cheby1.cpp \
    axislabel.cpp \
    glsw.cpp \
    utilities.cpp \
    audiohandle.cpp \
    glbase.cpp

HEADERS  += mainwindow.h \
    plotwidget.h \
    glospectrum.h \
    iir.h \
    plotfilter.h \
    cheby1.h \
    axislabel.h\
    glsw.h \
    utilities.h \
    audiohandle.h \
    glbase.h

INCLUDEPATH  += ../include

unix: LIBS += -lsndfile -lfftw3 -lsamplerate
win32: LIBS += ../lib/win32/sndfile.lib ../lib/win32/fftw3.lib ../lib/win32/samplerate.lib
