#include "plot.h"

#include<QLabel>
#include<QWheelEvent>

#include<iostream>

using namespace std;
plot::plot(float *ria,int num,float time,bool sampleOrNot,QWidget *parent) :
    QWidget(parent),widget1(NULL)
{
    widget=new GLSW(ria,num,time,sampleOrNot);
    if(sampleOrNot)
    {
        domain=new QLabel("Discrete-time domain");
        unit = new QLabel("    Time(sec)");
    }
    else
    {
        domain=new QLabel("Time domain");
        unit = new QLabel("Time(sec)");
    }
    initialCommonConp(true);
}

plot::plot(float *ria,int num,bool discrete,int sr,QWidget *parent)
    :QWidget(parent),widget(NULL)
{
    widget1=new GLOSpectrum(ria,num,discrete,sr);
    if(!discrete)
    {
        domain=new QLabel("   Frequency domain");
        unit = new QLabel("Frequency in Hz( *1000)");
    }
    else
    {
        domain=new QLabel("Discrete-time Fourier transform");
        unit = new QLabel("        Frequency");
    }
    initialCommonConp(false);
}

GLSW* plot::getGlwidget()
{
    return this->widget;
}

void plot::resetw()
{
    hSc->setSliderPosition(0);
}

GLOSpectrum* plot::getWidget()
{
    return this->widget1;
}

void plot::initialCommonConp(bool wave)
{
    hSc=new QScrollBar(Qt::Horizontal);
    hSc->setMinimum(-50);
    vSc=new QScrollBar;
    hSc->setMaximum(50);
    hSc->setSliderPosition(0);
    if(wave)
        vSc->setMinimum(-100);
    else
        vSc->setMinimum(0);
    vSc->setMaximum(100);

    vReset=new QToolButton;
    vReset->setText("R");
    vReset->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    vReset->setFixedSize(20,20);

    hReset=new QToolButton;
    hReset->setText("R");
    hReset->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    hReset->setFixedSize(20,20);

    vPlus=new QToolButton;
    vPlus->setText("+");
    vPlus->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    vPlus->setFixedSize(20,20);

    vMinus=new QToolButton;
    vMinus->setText("-");
    vMinus->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    vMinus->setFixedSize(20,20);

    hPlus=new QToolButton;
    hPlus->setText("+");
    hPlus->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    hPlus->setFixedSize(20,20);

    hMinus=new QToolButton;
    hMinus->setText("-");
    hMinus->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    hMinus->setFixedSize(20,20);

    if(wave)
    {
        connect(hSc,SIGNAL(valueChanged(int)),widget,SLOT(setHshift(int)));
        connect(vSc,SIGNAL(valueChanged(int)),widget,SLOT(setVshift(int)));
        connect(vReset,SIGNAL(clicked()),widget,SLOT(resetV()));
        connect(hReset,SIGNAL(clicked()),widget,SLOT(resetH()));
        connect(vPlus,SIGNAL(clicked()),widget,SLOT(vIncrease()));
        connect(vMinus,SIGNAL(clicked()),widget,SLOT(vDecrease()));
        connect(hPlus,SIGNAL(clicked()),widget,SLOT(hIncrease()));
        connect(hMinus,SIGNAL(clicked()),widget,SLOT(hDecrease()));    
    }
    else
    {
        connect(hSc,SIGNAL(valueChanged(int)),widget1,SLOT(setHshift(int)));
        connect(vSc,SIGNAL(valueChanged(int)),widget1,SLOT(setVshift(int)));
        connect(vReset,SIGNAL(clicked()),widget1,SLOT(resetV()));
        connect(hReset,SIGNAL(clicked()),widget1,SLOT(resetH()));
        connect(vPlus,SIGNAL(clicked()),widget1,SLOT(vIncrease()));
        connect(vMinus,SIGNAL(clicked()),widget1,SLOT(vDecrease()));
        connect(hPlus,SIGNAL(clicked()),widget1,SLOT(hIncrease()));
        connect(hMinus,SIGNAL(clicked()),widget1,SLOT(hDecrease()));
    }

    QGridLayout *ConpLayout = new QGridLayout;
    QHBoxLayout* horizontalLayout1 = new QHBoxLayout;
    horizontalLayout1->addWidget(hMinus);
    horizontalLayout1->addWidget(hSc);
    horizontalLayout1->addWidget(hPlus);
    horizontalLayout1->addWidget(hReset);
    horizontalLayout1->setAlignment(Qt::AlignHCenter);
    ConpLayout->addLayout(horizontalLayout1,0,0,1,10);
    ConpLayout->addWidget(vReset,8,14);
    ConpLayout->addWidget(unit,8,5);
    ConpLayout->addWidget(domain,1,5);
    if(wave)
    {
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ConpLayout->addWidget(widget,2,0,6,10);
    }
    else
    {
        widget1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ConpLayout->addWidget(widget1,2,0,6,10);
    }
    ConpLayout->addWidget(vPlus,1,14);
    ConpLayout->addWidget(vSc,2,14,5,1);
    ConpLayout->addWidget(vMinus,7,14);
    setLayout(ConpLayout);
}

