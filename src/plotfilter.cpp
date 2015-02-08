#include "plotfilter.h"

#include "glwidgetnew.h"
#include "glspectrum.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>
#include <QCloseEvent>
#include <QDebug>

plotFilter::plotFilter(float *ria,int num,bool discrete,int sr,QVector<float>& specdata,QWidget *parent) :
    QWidget(parent)
{
    this->sr=sr;
    name = new QLabel(this);
    font.setBold(true);
    font.setPointSize(10);
    name->setText("Anti-aliasing filter");
    name->setFont(font);
    float  time=(float)num/sr;
    QVector<float> datapt;
    for( int i = 0; i < num; i++ )
        datapt.push_back(ria[i]);

    _wavewidget = new GLWidgetnew(datapt,time,false);
    _wavewidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _specwidget = new GLSpectrum(specdata, discrete,1.0,sr);
    QHBoxLayout* widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(_wavewidget);
    widgetLayout->addWidget(_specwidget);
    Fdes=new QLabel(this);
    font.cleanup();
    font.setBold(true);
    Fdes->setFont(font);
    Fdes->setText("Filter description");
    filterSelect=new QComboBox(this);
    texts<<"Type:   Butterworth(IIR)"<<"Type:   Chebyshev I(IIR)";
    filterSelect->addItems(texts);
    order=new QLabel(this);
    order->setText("Order");
    numOfOrder=new QLineEdit(this);
    numOfOrder->setText("2");
    ripple=new QLineEdit(this);
    ripple->setText("0.5");
    rip=new QLabel(this);
    rip->setText("Ripple:");
    if((filterSelect->currentIndex()==-1)||(filterSelect->currentIndex()==0))
    {
        ripple->setVisible(false);
        rip->setVisible(false);
    }
    connect(filterSelect,SIGNAL(currentIndexChanged(int)),this,SLOT(filterChanged(int)));
    QHBoxLayout* orderL=new QHBoxLayout(this);
    OrderIsToHigh = new QLabel(this);
    OrderIsToHigh->setText("Order is too high!");
    OrderIsToHigh->setStyleSheet("QLabel {  color : red; }");
    OrderIsToHigh->setVisible(false);
    RippleNotValid = new QLabel(this);
    RippleNotValid->setText("Ripple is not valid");
    RippleNotValid->setStyleSheet("QLabel {  color : red; }");
    RippleNotValid->setVisible(false);
    orderL->addWidget(order);
    orderL->addWidget(numOfOrder);
    orderL->addWidget(rip);
    orderL->addWidget(ripple);
    orderL->addWidget(OrderIsToHigh);
    orderL->addWidget(RippleNotValid);
    ooder=new QWidget(this);
    ooder->setLayout(orderL);
    cutOfF=new QLabel(this);
    cutOfF->setText("Cutoff frequency");
    cutFreqSel=new QComboBox(this);
    texts.clear();
   /* if(sr==11025)
    {
        texts<<QString::number(11025)<<QString::number(10000)<<QString::number(5000)<<QString::number(2500)<<
               QString::number(2000)<<QString::number(1000)<<
               QString::number(500)<<QString::number(250)<<
               QString::number(200)<<QString::number(100);
    }
    if(sr == 44100)
    {
        texts<<QString::number(sr)<<QString::number(40000)<<QString::number(30000)<<
               QString::number(20000)<<QString::number(15000)<<QString::number(10000)<<QString::number(5000)<<QString::number(2500)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<QString::number(250)<<
               QString::number(200)<<QString::number(100);
    }*/
    texts<<QString::number(sr)<<QString::number(sr/2)<<QString::number(sr/3)<<
            QString::number(sr/4)<<QString::number(sr/5)<<QString::number(sr/6)<<
            QString::number(sr/7)<<QString::number(sr/8)<<QString::number(sr/9)<<
            QString::number(sr/10)<<QString::number(sr/20);
    cutFreqSel->addItems(texts);
    QHBoxLayout *activeFreqLayout = new QHBoxLayout;
    reset=new QToolButton(this);
    reset->setText("Set to Nyq. Freq.");
    QLabel *activeLabel = new QLabel("Actual cut off freq.:");
    _displayActualFreq = new QLabel(cutFreqSel->currentText());
    activeFreqLayout->addWidget(reset);
    activeFreqLayout->addStretch();
    activeFreqLayout->addWidget(activeLabel);
    activeFreqLayout->addWidget(_displayActualFreq);
    play=new QToolButton(this);
    play->setText("Play");
    apply=new QToolButton(this);
    apply->setText("Apply");
    connect(apply,SIGNAL(clicked()),this,SLOT(setActualFreq()));
    connect(reset,SIGNAL(clicked()),this,SLOT(setCutFreqToNy()));
    connect(reset,SIGNAL(clicked()),this,SLOT(setActualFreq()));
    QGridLayout *ConpLayout = new QGridLayout(this);
    ConpLayout->addWidget(name,0,0);
    ConpLayout->addLayout(widgetLayout,1,0);
    ConpLayout->addWidget(Fdes,2,0);
    ConpLayout->addWidget(filterSelect,3,0);
    ConpLayout->addWidget(ooder,4,0);
    ConpLayout->addWidget(cutOfF,5,0);
    ConpLayout->addWidget(cutFreqSel,6,0);
    ConpLayout->addLayout(activeFreqLayout,7,0);
    ConpLayout->addWidget(play,8,1);
    ConpLayout->addWidget(apply,8,0);
    this->setLayout(ConpLayout);
}

void plotFilter::filterChanged(int index)
{
    if((index==-1)||(index==0))
    {
        ripple->setVisible(false);
        rip->setVisible(false);
    }
    else
    {
        ripple->setVisible(true);
        rip->setVisible(true);
    }
}

void plotFilter::setComboText(QString string)
{
 NqLabel = string;
 int b;
 if(NqLabel.toInt()==11025 || NqLabel.toInt() == 44100)
 {
      b = NqLabel.toInt();
 }
 else
  b = NqLabel.toInt() / 2;
 QString s = QString::number(b);
 int a = texts.indexOf(s);
 if(a==-1)
     return;
  cutFreqSel->setCurrentIndex(a);
}

void plotFilter::setCutFreqToNy()
{
  int b = NqLabel.toInt() / 2;
  QString s = QString::number(b);
  int a = texts.indexOf(s);
  if(a==-1)
      return;
   cutFreqSel->setCurrentIndex(a);
}

void plotFilter::closeEvent(QCloseEvent *event)
{
  event->accept();
  emit closed(false);
}

void plotFilter::setActualFreq()
{
  _displayActualFreq->setText(cutFreqSel->currentText());
}

GLWidgetnew* plotFilter::getWaveWidget()
{
    return _wavewidget;
}

QComboBox* plotFilter::getFilterSelect()
{
    return filterSelect;
}

QLineEdit* plotFilter::getNumOfOrder()
{
    return numOfOrder;
}

QLineEdit* plotFilter::getRippleEdit()
{
    return ripple;
}

QLabel* plotFilter::getOrderLabel()
{
    return OrderIsToHigh;
}

QLabel* plotFilter::getRippleLabel()
{
    return RippleNotValid;
}

QComboBox* plotFilter::getCutFreq()
{
    return cutFreqSel;
}

QToolButton* plotFilter::getSetTonyqB()
{
    return reset;
}

GLSpectrum* plotFilter::getSpecWidget()
{
    return _specwidget;
}

QToolButton* plotFilter::getApplyButton()
{
    return apply;
}
