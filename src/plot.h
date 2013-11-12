#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include<QGLWidget>
#include<QScrollBar>
#include"glsw.h"
#include<QToolButton>
#include"glospectrum.h"
#include<QLabel>
#include<QGridLayout>
class plot : public QWidget
{
    Q_OBJECT
public:
 plot(float *ria,int num,float time,bool sampleOrnot,QWidget *parent = 0);
 plot(float *ria,int num,bool discrete,int sr,QWidget *parent = 0);
 GLSW* widget;
 GLSW* getGlwidget();
 GLOSpectrum* widget1;
 GLOSpectrum* getWidget();

 QScrollBar *hSc;
 QScrollBar *vSc;
 QToolButton *vPlus,*vMinus,*vReset,*hPlus,*hMinus,*hReset;
 QLabel *domain;
 QLabel* unit;
signals:

public slots:
 void resetw();

private:
 void initialCommonConp(bool);

};

#endif // PLOT_H
