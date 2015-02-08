#include "plotwidget.h"

#include "glwidgetnew.h"
#include "glspectrum.h"

#include <QLabel>
#include <QWheelEvent>
#include <QToolButton>
#include <QLabel>
#include <QGridLayout>
#include <QScrollBar>

#include<iostream>
#include <QDebug>

PlotWidget::PlotWidget(float *ria,int num,float time,bool sampleOrNot,QWidget *parent) :
    QWidget(parent),_spectrumWidget(NULL)
{
    QVector<float> datapt;
    for( int i = 0; i < num; i++ )
        datapt.push_back(ria[i]);
    _wavewidget = new GLWidgetnew(datapt,time,sampleOrNot);
    if(sampleOrNot)
    {
        domain=new QLabel("Discrete-time domain");
        unit = new QLabel("    Time (sec)");
    }
    else
    {
        domain=new QLabel("Time domain");
        unit = new QLabel("Time (sec)");
    }
    initialCommonConp(true);
}

PlotWidget::PlotWidget(const QVector<float>& data,bool discrete,int sr,QWidget *parent)
    :QWidget(parent),_wavewidget(NULL)
{
    if(!discrete)
    {
        domain=new QLabel("   Frequency domain");
        unit = new QLabel("Frequency in kHz");
        _spectrumWidget = new GLSpectrum(data,discrete,1.0,sr);
    }
    else
    {
        _spectrumWidget = new GLSpectrum(data,discrete,1.0,sr);
        domain=new QLabel("Discrete-time Fourier transform");
        unit = new QLabel("        Frequency (Pi)");
    }
    initialCommonConp(false);
}


GLWidgetnew* PlotWidget::getWaveWidget()
{
    return this->_wavewidget;
}

GLSpectrum* PlotWidget::getSpecWidget()
{
    return _spectrumWidget;
}

void PlotWidget::initialCommonConp(bool wave)
{

    hReset=new QToolButton;
    hReset->setText("R");
    hReset->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    hReset->setFixedSize(20,20);
    QGridLayout *ConpLayout = new QGridLayout;
    if(wave)
    {
        connect(hReset,SIGNAL(clicked()),_wavewidget,SLOT(resetH()));
    }
    else
    {
        if(_spectrumWidget->getIsSample())
        {
            QToolButton* _yplus = new QToolButton;
            _yplus->setText("Y+");
            _yplus->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            _yplus->setFixedSize(20,20);
            QToolButton* _yminus = new QToolButton;
            _yminus->setText("Y-");
            _yminus->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            _yminus->setFixedSize(20,20);
            connect(_yplus,SIGNAL(clicked()),_spectrumWidget,SLOT(vIncrease()));
            connect(_yminus,SIGNAL(clicked()),_spectrumWidget,SLOT(vDecrease()));
            ConpLayout->addWidget(_yplus,1,7);
            ConpLayout->addWidget(_yminus,1,8);
        }
        connect(hReset,SIGNAL(clicked()),_spectrumWidget,SLOT(resetH()));
    }
    ConpLayout->addWidget(domain,1,5);
    ConpLayout->addWidget(hReset,1,6);

    if(wave)
    {
        _wavewidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ConpLayout->addWidget(_wavewidget,2,0,6,10);
    }
    else
    {
        _spectrumWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ConpLayout->addWidget(_spectrumWidget,2,0,6,10);
    }
    ConpLayout->addWidget(unit,9,5);
    setLayout(ConpLayout);
}

