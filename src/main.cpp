#include <QApplication>
#include <QSurfaceFormat>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   a.setApplicationName("DSP Demo-uni konstanz");

   QSurfaceFormat  format;

   format.setSamples(4);
   format.setDepthBufferSize(24);
   format.setStencilBufferSize(8);
   //format.setVersion(3,2);
   //format.setProfile(QSurfaceFormat::CoreProfile);
   QSurfaceFormat::setDefaultFormat(format);

   MainWindow *w = new MainWindow();
   w->show();

   return a.exec();
}
