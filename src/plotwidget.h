#ifndef PLOT_H
#define PLOT_H

#include <QWidget>

class QScrollBar;
class QToolButton;
class QLabel;

class GLSW;
class GLOSpectrum;

class PlotWidget : public QWidget
{
  Q_OBJECT
	
  public:
   PlotWidget(float *ria,int num,float time,bool sampleOrnot,QWidget *parent = 0);
   PlotWidget(float *ria,int num,bool discrete,int sr,QWidget *parent = 0);
   GLSW* widget;
   GLSW* getGlwidget();
   GLOSpectrum* widget1;
   GLOSpectrum* getWidget();
   QScrollBar *hSc;
   QScrollBar *vSc;
   QToolButton *vPlus,*vMinus,*vReset,*hPlus,*hMinus,*hReset;
   QLabel *domain;
   QLabel* unit;
 
  public slots:
    void resetw();

  private:
    void initialCommonConp(bool);

};

#endif // PLOT_H