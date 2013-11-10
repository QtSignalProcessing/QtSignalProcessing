#-------------------------------------------------
#
# Project created by QtCreator 2013-01-29T16:08:41
#
#-------------------------------------------------

QT += core gui opengl

TARGET = SignalProcessing
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    plot.cpp \
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
    plot.h \
    glospectrum.h \
    iir.h \
    plotfilter.h \
    cheby1.h \
    axislabel.h\
    glsw.h \
    utilities.h \
    audiohandle.h \
    glbase.h
unix|win32: LIBS += -lsndfile

unix|win32: LIBS += -lfftw3
QT += phonon

unix|win32: LIBS += -lsamplerate
