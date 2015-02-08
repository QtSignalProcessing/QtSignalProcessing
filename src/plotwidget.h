#ifndef PLOT_H
#define PLOT_H

#include <QWidget>

class QScrollBar;
class QToolButton;
class QLabel;

class GLWidgetnew;
class GLSpectrum;

class PlotWidget : public QWidget
{
  Q_OBJECT
	
  public:
   PlotWidget(float *ria,int num,float time,bool sampleOrnot,QWidget *parent = 0);
   PlotWidget(const QVector<float>& data,bool discrete,int sr,QWidget *parent = 0);
   GLWidgetnew* _wavewidget;
   GLWidgetnew* getWaveWidget();
   GLSpectrum* _spectrumWidget;
   GLSpectrum* getSpecWidget();
   QToolButton *hReset;
   QLabel *domain;
   QLabel* unit;
 
  public slots:

  private:
    void initialCommonConp(bool);

};

#endif // PLOT_H
