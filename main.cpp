#include <QtGui/QApplication>
#include "mainwindow.h"
#include"glwidget.h"
#include"plot.h"
#include"plotospectrum.h"
#include"plotsw.h"
#include"glsw.h"
#include<iostream>
#include<QDoubleSpinBox>
#include<QGridLayout>
#include <QDebug>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include<math.h>
#include"iir.h"
#include"xwilkinson.h"



using namespace Phonon;

using namespace std;
int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MainWindow *w=new MainWindow();
   w->show();
   return a.exec();
}
