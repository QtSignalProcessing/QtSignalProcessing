#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   a.setApplicationName("DSP Demo-uni konstanz");
   MainWindow *w = new MainWindow();
   w->show();

   return a.exec();
}
