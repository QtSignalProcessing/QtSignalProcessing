#include "mainwindow.h"

#include "plotwidget.h"
#include "utilities.h"
#include "audiohandle.h"
#include "plotfilter.h"
#include "glwidgetnew.h"
#include "glspectrum.h"

#include <QMouseEvent>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QMenu>
#include <QLabel>
#include <QToolButton>
#include <QMessageBox>
#include <QMenuBar>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QApplication>
#include <QAudioOutput>

#if QT_VERSION < 0x050000
#include <phonon/MediaObject>
#else
#include <QMediaPlayer>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),_data(nullptr),_quantizedData(nullptr),bits(0),tmpData(nullptr),ifShowAlias(false),
     _trueBits(0),_selFilter(nullptr),_showAliasing(nullptr),_showSampleRange(nullptr),_showOnePeriod(nullptr),loadingFailed(false),_orgData(nullptr),
      _orgFileName(),_sampleFileName(),_filteredFileName(),_samplingRates(),_filterData(),_sampleData()
{
    createActions();
    createMenus();
    ria = new AudioHandle(QString("NULL"));
    _orgFileName = "helloDSP.wav";
    main=new QWidget(this);
    this->setCentralWidget(main);
    createWidget(main);
    this->setWindowTitle("DSP Demo - University of Konstanz");
}

void MainWindow::initializeVar()
{
    _filterData.ripple = 0.5;
    _filterData.butter = true;
    _filterData.order = 2;
    _sampleData.upSampleFactor = 1;
    _sampleData.downSampleFactor = 1;
    int f = ria->getFrameNum();
    sr=ria->getSamplerate();
    int c=ria->getChannel();
    int num_items = f*c;
    time=(float)num_items/sr;
    if(_data != nullptr)
        delete[] _data;
    _data=ria->getData(num_items,num);
    if(_data)
    {
        _sampleData.currentNum = num;
        loadingFailed = false;
        if(_orgData!=nullptr)
            delete _orgData;
        _orgData = new float[num*sizeof(float)];
        for(int i =0; i < num; i++)
        {
            _orgData[i] =_data[i] ;
        }
    }
    else
    {
        num = 20000;
        _sampleData.currentNum=num;
        _data = new float[sizeof(float)*num];
        sr = 11025;
        for(int i = 0; i< 20000;i++)
            _data[i] = (sin(i*3.14/2));
        _orgData = new float[sizeof(float)*num];
        for(int i =0; i < num; i++)
            _orgData[i] =_data[i] ;
        loadingFailed = true;
        _orgFileName=QDir::tempPath()+"/tmp.wav";
        ria->writeToWave(_data,_orgFileName.toLatin1().data(),sr,num);
    }
}

void MainWindow::createWidget(QWidget *main)
{
     initializeVar();
    utilities=new Utilities(_data,num);
    _SampledWave=new PlotWidget(_data,num,time,true,main);
    _OrgWave=new PlotWidget(_data,num,time,false,main);
    QVector<float> spec = utilities->getAmplitude();
    _ConSpec=new PlotWidget(spec,false,sr,main);
    _DisSpec=new PlotWidget(spec,true,sr,main);
    _OrgWave->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _ConSpec->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _SampledWave->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _DisSpec->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(_OrgWave->getWaveWidget(),SIGNAL(xAxisScaled(double)),_SampledWave->getWaveWidget(),SLOT(setLinkedScale(double)));
    connect(_SampledWave->getWaveWidget(),SIGNAL(xAxisScaled(double)),_OrgWave->getWaveWidget(),SLOT(setLinkedScale(double)));
    connect(_OrgWave->getWaveWidget(),SIGNAL(xAxisShifted(double)),_SampledWave->getWaveWidget(),SLOT(setLinkedShift(double)));
    connect(_SampledWave->getWaveWidget(),SIGNAL(xAxisShifted(double)),_OrgWave->getWaveWidget(),SLOT(setLinkedShift(double)));
    connect(_ConSpec->getSpecWidget(),SIGNAL(xAxisScaled(double)),_DisSpec->getSpecWidget(),SLOT(setLinkedScale(double)));
    connect(_DisSpec->getSpecWidget(),SIGNAL(xAxisScaled(double)),_ConSpec->getSpecWidget(),SLOT(setLinkedScale(double)));
    connect(_ConSpec->getSpecWidget(),SIGNAL(xAxisShifted(double)),_DisSpec->getSpecWidget(),SLOT(setLinkedShift(double)));
    connect(_DisSpec->getSpecWidget(),SIGNAL(xAxisShifted(double)),_ConSpec->getSpecWidget(),SLOT(setLinkedShift(double)));
    connect(_OrgWave->hReset,SIGNAL(clicked()),_SampledWave->getWaveWidget(),SLOT(resetH()));
    connect(_SampledWave->hReset,SIGNAL(clicked()),_OrgWave->getWaveWidget(),SLOT(resetH()));
    connect(_SampledWave->hReset,SIGNAL(clicked()),_SampledWave->getWaveWidget(),SLOT(resetH()));
    connect(_OrgWave->hReset,SIGNAL(clicked()),_OrgWave->getWaveWidget(),SLOT(resetH()));
    QGridLayout *ConpLayout = new QGridLayout(main);
    ConpLayout->addWidget(_OrgWave,0,0);
    ConpLayout->addWidget(_ConSpec,0,2);
    ConpLayout->addWidget(_SampledWave,2,0);
    ConpLayout->addWidget(_DisSpec,2,2);
    QToolButton *playB=new QToolButton(main);
    playB->setText("play");
    playB->connect(playB,SIGNAL(clicked()),this,SLOT(play()));
    ConpLayout->addWidget(playB,1,0);
    QToolButton *play1=new QToolButton(main);
    play1->setText("play");
    play1->connect(play1,SIGNAL(clicked()),this,SLOT(playSample()));
    QCheckBox* stickOn = new QCheckBox("Stick on",this);
    connect(stickOn,SIGNAL(toggled(bool)),_SampledWave->getWaveWidget(),SLOT(setStickOn(bool)));
    QHBoxLayout* bottomLeft = new QHBoxLayout;
    bottomLeft->addWidget(play1);
    bottomLeft->addWidget(stickOn);
    ConpLayout->addLayout(bottomLeft,3,0);
    QWidget* rightWidget=new QWidget(main);
    QVBoxLayout *vLayout=new QVBoxLayout;
    //quantization
    if(loadingFailed)
    {
        _trueBits = 16;
    }
    else
    {
        _trueBits = utilities->computeTrueBits();
    }
    bitBox=new QSpinBox(main);
    bitBox->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    bitBox->setValue(_trueBits);
    bitBox->setMaximum(_trueBits);
    bitBox->setMinimum(1);
    connect(bitBox,SIGNAL(valueChanged(int)),this,SLOT(setBits(int)));
    QLabel* quan=new QLabel("Quantization",this);
    QFont font;
    font.setBold(true);
    quan->setFont(font);
    QLabel *title=new QLabel("Number of bits(B+1)",this);
    QLabel* title1=new QLabel("Sample original signal at rate:",this);
    title1->setFont(font);
    sampleRateSelect=new QComboBox(main);
    QStringList texts;
    if(sr==11025)
    {
        /*
    texts<<QString::number(sr)<<QString::number(10000)<<QString::number(5000)<<
           QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
           QString::number(200)<<QString::number(100);*/
        texts<<QString::number(sr)+" Max Sampling rate"<<QString::number(sr/2)+" 1/2 of max Sampling rate"<<QString::number(sr/3)+" 1/3 of max Sampling rate"<<
                QString::number(sr/4)+" 1/4 of max Sampling rate"<<QString::number(sr/5)+" 1/5 of max Sampling rate"<<QString::number(sr/6)+" 1/6 of max Sampling rate"<<
                QString::number(sr/7)+" 1/7 of max Sampling rate"<<QString::number(sr/8)+" 1/8 of max Sampling rate"<<QString::number(sr/9)+" 1/9 of max Sampling rate"<<
                QString::number(sr/10)+" 1/10 of max Sampling rate"<<QString::number(sr/20)+" 1/20 of max Sampling rate";
    }
    if(sr == 44100)
    {
        /*texts<<QString::number(sr)<<QString::number(40000)<<QString::number(30000)<<
               QString::number(20000)<<QString::number(10000)<<QString::number(5000)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
               QString::number(200)<<QString::number(100)<<QString::number(18000);*/
        texts<<QString::number(sr)+" Max Sampling rate"<<QString::number(sr/2)+" 1/2 of max Sampling rate"<<QString::number(sr/3)+" 1/3 of max Sampling rate"<<
                QString::number(sr/4)+" 1/4 of max Sampling rate"<<QString::number(sr/5)+" 1/5 of max Sampling rate"<<QString::number(sr/6)+" 1/6 of max Sampling rate"<<
                QString::number(sr/7)+" 1/7 of max Sampling rate"<<QString::number(sr/8)+" 1/8 of max Sampling rate"<<QString::number(sr/9)+" 1/9 of max Sampling rate"<<
                QString::number(sr/10)+" 1/10 of max Sampling rate"<<QString::number(sr/20)+" 1/20 of max Sampling rate";
    }
    for(int i=1;i<=10;i++)
    {
        int k = sr/i;
        _samplingRates.append(k);
    }
    int k = sr/20;
    _samplingRates.append(k);
    sampleRateSelect->addItems(texts);
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(int)),this,SLOT(nonIntSr(int)));

    QHBoxLayout* h2=new QHBoxLayout;
    QLabel* showSNR=new QLabel("SNR",this);
    SNR=new QLabel(this);
    SNR->setText("inf");
    QWidget* snr=new QWidget(main);
    h2->addWidget(showSNR);
    h2->addWidget(SNR);
    snr->setLayout(h2);
    QVBoxLayout* h1=new QVBoxLayout;
    _showSampleRange=new QCheckBox("Show max. Nyquist freq.",main);
    _showOnePeriod = new QCheckBox("Show only one period",main);
    connect(_showOnePeriod,SIGNAL(clicked(bool)),_DisSpec->getSpecWidget(),SLOT(show1Period(bool)));
    connect(_showSampleRange,SIGNAL(stateChanged(int)),_ConSpec->getSpecWidget(),SLOT(showSampleRange(int)));
    connect(_showSampleRange,SIGNAL(stateChanged(int)),_DisSpec->getSpecWidget(),SLOT(showSampleRange(int)));
    h1->addWidget(_showSampleRange);
    h1->addWidget(_showOnePeriod);
    QWidget* samplerange=new QWidget(main);
    samplerange->setLayout(h1);
    QGridLayout* v1=new QGridLayout;
    QLabel* filterLabel=new QLabel("Apply anti-aliasing filter",this);
    _addNoise = new QCheckBox("Add sinus wave to signal",this);
    connect(_addNoise,SIGNAL(toggled(bool)),this,SLOT(enableNoiseSelect(bool)));
    _selectNoise = new QComboBox(this);
    _selectNoise->setEnabled(false);

    texts.clear();
    texts<<QString::number(sr/4)<<QString::number(sr/2)<<QString::number(3*sr/4);
    _selectNoise->addItems(texts);
    filterLabel->setFont(font);
    QWidget* ffiltter =new QWidget(main);
    _selFilter=new QCheckBox("Apply filter",main);
    _showAliasing = new QCheckBox("Show alasing",main);
    connect(_selFilter,SIGNAL(clicked(bool)),this,SLOT(showFilter(bool)));
    connect(_showAliasing,SIGNAL(clicked(bool)),this,SLOT(displayAliasing(bool)));
    v1->setSpacing(0);
    QVBoxLayout* noiseVBoxLayout = new QVBoxLayout;
    noiseVBoxLayout->addWidget(_addNoise);
    noiseVBoxLayout->addWidget(_selectNoise);
    connect(_selectNoise,SIGNAL(currentIndexChanged(int)),this,SLOT(noiseAddfunc(int)));
    v1->addLayout(noiseVBoxLayout,0,0);
    v1->addWidget(filterLabel,1,0);
    v1->addWidget(_selFilter,2,0);
    v1->addWidget(_showAliasing,3,0);
    ffiltter->setLayout(v1);
    ConpLayout->addWidget(ffiltter,0,3);
    vLayout->setSpacing(0);
    vLayout->addWidget(title1);
    vLayout->addWidget(sampleRateSelect);
    vLayout->addWidget(samplerange);
    vLayout->addWidget(quan);
    QHBoxLayout* h3=new QHBoxLayout;
    h3->addWidget(title);
    h3->addWidget(bitBox);
    QWidget* showquan=new QWidget(main);
    showquan->setLayout(h3);
    vLayout->addWidget(showquan);
    vLayout->addWidget(snr);
    show2Spec=new QCheckBox("Show 2 freq.");
    connect(show2Spec,SIGNAL(clicked()),this,SLOT(plot2Freq()));
    connect(show2Spec,SIGNAL(clicked(bool)),_DisSpec->getSpecWidget(),SLOT(show2F(bool)));
    conFirst=new QCheckBox("Cont. first",main);
    disFirst=new QCheckBox("Discrete first",main);
    conFirst->setDisabled(true);
    disFirst->setDisabled(true);
    connect(conFirst,SIGNAL(clicked()),this,SLOT(onlyCon()));
    connect(conFirst,SIGNAL(toggled(bool)),_DisSpec->getSpecWidget(),SLOT(conDis(bool)));
    connect(disFirst,SIGNAL(clicked()),this,SLOT(onlyDis()));
    connect(disFirst,SIGNAL(toggled(bool)),_DisSpec->getSpecWidget(),SLOT(disCon(bool)));
    QHBoxLayout* h4=new QHBoxLayout;
    h4->addWidget(conFirst);
    h4->addWidget(disFirst);
    QWidget* _show2Spec=new QWidget(main);
    _show2Spec->setLayout(h4);
    vLayout->addWidget(show2Spec);
    vLayout->addWidget(_show2Spec);
    connect(_SampledWave->getWaveWidget(),SIGNAL(newSNR(double)),SNR,SLOT(setNum(double)));
    rightWidget->setLayout(vLayout);
    ConpLayout->addWidget(rightWidget,2,3);
    _FilterWidget=new plotFilter(_data,num,false,sr,spec,0);
    _FilterWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
   // connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_FilterWidget,SLOT(setComboText(QString))); //TODO
    connect(sampleRateSelect,SIGNAL(currentTextChanged(QString)),_FilterWidget,SLOT(setComboText(QString)));
    _FilterWidget->hide();
    connect(_FilterWidget->play,SIGNAL(clicked()),this,SLOT(playfiltered()));
    connect(_FilterWidget,SIGNAL(closed(bool)),this,SLOT(showFilter(bool)));
    connect(_FilterWidget->getFilterSelect(),SIGNAL(currentIndexChanged(int)),this,SLOT(butterOrCheby(int)));
    connect(_FilterWidget->getNumOfOrder(),SIGNAL(textEdited(QString)),this,SLOT(setOrder(QString)));
    connect(_FilterWidget->getRippleEdit(),SIGNAL(textEdited(QString)),this,SLOT(setRipple(QString)));
    connect(_FilterWidget->getSetTonyqB(),SIGNAL(clicked()),this,SLOT(setNyqFreq()));
    connect(_FilterWidget->getApplyButton(),SIGNAL(clicked()),this,SLOT(applyFilters()));
  //  connect(_FilterWidget->getCutFreq(),SIGNAL(currentIndexChanged(QString)),this,SLOT(setFactor(QString)));// TODO
    connect(_FilterWidget->getCutFreq(),SIGNAL(currentTextChanged(QString)),this,SLOT(setFactor(QString)));
}

MainWindow::~MainWindow(){}

void MainWindow::play()
{
#if QT_VERSION < 0x050000
  Phonon::MediaObject* player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_orgFileName));
#else
  QMediaPlayer* player = new QMediaPlayer();
  player->setSource(QUrl::fromLocalFile(_orgFileName));
  QAudioOutput* audioOutput = new QAudioOutput;
  player->setAudioOutput(audioOutput);
#endif
  player->play();
}

void MainWindow::playSample(){
#if QT_VERSION < 0x050000
  Phonon::MediaObject* player;
  if (_sampleFileName == nullptr)
  {
    player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_orgFileName));
  }
  else
  {
    player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_sampleFileName));
  }
#else
  QMediaPlayer* player = new QMediaPlayer();
  if (_sampleFileName.isNull())
  {
      player->setSource(QUrl::fromLocalFile(_orgFileName));
  }
  else
  {
        player->setSource(QUrl::fromLocalFile(_sampleFileName));
  }
#endif
  QAudioOutput* audioOutput = new QAudioOutput;
  player->setAudioOutput(audioOutput);
  player->play();
}

void MainWindow::playfiltered()
{
  if(!_filteredFileName.isNull())
  {
#if QT_VERSION < 0x050000
  Phonon::MediaObject* player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_filteredFileName));
#else
  QMediaPlayer* player = new QMediaPlayer();
  player->setSource(QUrl::fromLocalFile(_filteredFileName));
#endif
  QAudioOutput* audioOutput = new QAudioOutput;
  player->setAudioOutput(audioOutput);
  player->play();
  }
}

void MainWindow::open()
{
        _orgFileName = QFileDialog::getOpenFileName(this);
        if (!_orgFileName.isEmpty())
            loadFile(_orgFileName);
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}

void MainWindow::aboutDefaultSignal()
{
    QMessageBox::about(this, tr("About Application"),
                       tr("The default signal is sin(0.1*n),the sampling rate(11025 Hz) is normalized to 2PI, so the frequency 0.1 in rad/s is 11025/2PI*0.1=176 Hz"));
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    aboutDefaultSignalAct =new QAction(tr("&About Default Signal"), this);
    connect(aboutDefaultSignalAct,SIGNAL(triggered()), this, SLOT(aboutDefaultSignal()));
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(exitAct);
    menuBar()->addSeparator();
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutDefaultSignalAct);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

//load in the selected audio file(need to add the filtering of the file types)
void MainWindow::loadFile(const QString &fileName)
{
    if(ria!=NULL)
        delete ria;
    ria = new AudioHandle(fileName);
    _sampleFileName = "";
    initializeVar();
    QStringList texts;
   if(sr==11025)
    {
       /* texts<<QString::number(sr)<<QString::number(10000)<<QString::number(5000)<<QString::number(4800)<<QString::number(4500)<<QString::number(sr/3)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
               QString::number(200)<<QString::number(100);*/
       texts<<QString::number(sr)+" Max Sampling rate"<<QString::number(sr/2)+" 1/2 of max Sampling rate"<<QString::number(sr/3)+" 1/3 of max Sampling rate"<<
              QString::number(sr/4)+" 1/4 of max Sampling rate"<<QString::number(sr/5)+" 1/5 of max Sampling rate"<<QString::number(sr/6)+" 1/6 of max Sampling rate"<<
              QString::number(sr/7)+" 1/7 of max Sampling rate"<<QString::number(sr/8)+" 1/8 of max Sampling rate"<<QString::number(sr/9)+" 1/9 of max Sampling rate"<<
              QString::number(sr/10)+" 1/10 of max Sampling rate"<<QString::number(sr/20)+" 1/20 of max Sampling rate";
    }
    else if(sr == 44100)
    {
       /* texts<<QString::number(sr)<<QString::number(40000)<<QString::number(30000)<<QString::number(sr/2)<<
               QString::number(20000)<<QString::number(18000)<<QString::number(15000)<<QString::number(10000)<<QString::number(8000)<<QString::number(6000)<<QString::number(5000)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
               QString::number(200)<<QString::number(100);*/
       texts<<QString::number(sr)+" Max Sampling rate"<<QString::number(sr/2)+" 1/2 of max Sampling rate"<<QString::number(sr/3)+" 1/3 of max Sampling rate"<<
              QString::number(sr/4)+" 1/4 of max Sampling rate"<<QString::number(sr/5)+" 1/5 of max Sampling rate"<<QString::number(sr/6)+" 1/6 of max Sampling rate"<<
              QString::number(sr/7)+" 1/7 of max Sampling rate"<<QString::number(sr/8)+" 1/8 of max Sampling rate"<<QString::number(sr/9)+" 1/9 of max Sampling rate"<<
              QString::number(sr/10)+" 1/10 of max Sampling rate"<<QString::number(sr/20)+" 1/20 of max Sampling rate";
    }
    _samplingRates.clear();
    for(int i=1;i<=10;i++)
    {
       int k = sr/i;
       _samplingRates.append(k);
    }
    int k = sr/20;
    _samplingRates.append(k);
    sampleRateSelect->blockSignals(true);
    sampleRateSelect->clear();
    sampleRateSelect->addItems(texts);
    sampleRateSelect->blockSignals(false);
    QVector<float> dpt = pointer2Qvec(_data,num);
    _OrgWave->getWaveWidget()->setTime(time);
    _OrgWave->getWaveWidget()->setData(dpt);
    _OrgWave->getWaveWidget()->updateMax();
    _OrgWave->getWaveWidget()->resetH();

    _SampledWave->getWaveWidget()->setTime(time);
    _SampledWave->getWaveWidget()->setData(dpt);
    _SampledWave->getWaveWidget()->updateMax();
    _SampledWave->getWaveWidget()->setOffset(1);
    _SampledWave->getWaveWidget()->resetH();

    QVector<float> tmps = utilities->getAmplitude(dpt);
    _ConSpec->getSpecWidget()->setData(tmps);
    _ConSpec->getSpecWidget()->updateMax();
    _ConSpec->getSpecWidget()->setRatio(1.0);
    _ConSpec->getSpecWidget()->updateSampleRate(sr);
    _ConSpec->getSpecWidget()->showSampleRange(0);
    _ConSpec->getSpecWidget()->resetH();

    _DisSpec->getSpecWidget()->setData(tmps);
    _DisSpec->getSpecWidget()->updateMax();
    _DisSpec->getSpecWidget()->setRatio(1.0);
    _DisSpec->getSpecWidget()->updateSampleRate(sr);
    _DisSpec->getSpecWidget()->showAliasing(false);
    _DisSpec->getSpecWidget()->showSampleRange(0);
    _DisSpec->getSpecWidget()->show1Period(false);
    _DisSpec->getSpecWidget()->show2F(false);
    _DisSpec->getSpecWidget()->resetH();

    _FilterWidget->getWaveWidget()->setData(dpt);
    _FilterWidget->getWaveWidget()->updateMax();
    _FilterWidget->getWaveWidget()->resetH();
    _FilterWidget->getSpecWidget()->setData(tmps);
    _FilterWidget->getSpecWidget()->updateMax();
    _FilterWidget->getSpecWidget()->setRatio(1.0);
    _FilterWidget->getSpecWidget()->updateSampleRate(sr);
    updateFilter();
    utilities->updateUtilites(_data,num);
    if(loadingFailed)
    {
       _trueBits = 16;
    }
    else
    {
    _trueBits = utilities->computeTrueBits();
    }
    bitBox->setMaximum(_trueBits);
    bitBox->setValue(_trueBits);
    showFilter(false);
    resetEverything();
    if(loadingFailed)
    {
       QMessageBox error;
       error.setText("Default audio file open failed, and sin signal with noise will be shown");
       error.exec();
    }
    _addNoise->setChecked(false);
    _selectNoise->setEnabled(false);
    _selectNoise->blockSignals(true);
    _selectNoise->clear();
    texts.clear();
    texts<<QString::number(sr/4)<<QString::number(sr/2)<<QString::number(3*sr/4);
    _selectNoise->addItems(texts);
    _selectNoise->blockSignals(false);
}

void MainWindow::updateFilter()
{
    QStringList texts;
    if(sr==11025)
    {
        texts<<QString::number(11025)<<QString::number(10000)<<QString::number(5000)<<QString::number(2500)<<
               QString::number(2000)<<QString::number(1000)<<
               QString::number(500)<<QString::number(250)<<
               QString::number(200)<<QString::number(100);
       /* texts<<QString::number(sr)+" Max Sampling rate"<<QString::number(sr/2)+" 1/2 of max Sampling rate"<<QString::number(sr/3)+" 1/3 of max Sampling rate"<<
                QString::number(sr/4)+" 1/4 of max Sampling rate"<<QString::number(sr/5)+" 1/5 of max Sampling rate"<<QString::number(sr/6)+" 1/6 of max Sampling rate"<<
                QString::number(sr/7)+" 1/7 of max Sampling rate"<<QString::number(sr/8)+" 1/8 of max Sampling rate"<<QString::number(sr/9)+" 1/9 of max Sampling rate"<<
                QString::number(sr/10)+" 1/10 of max Sampling rate"<<QString::number(sr/20)+" 1/20 of max Sampling rate";*/
    }
    if(sr == 44100)
    {
        texts<<QString::number(sr)<<QString::number(40000)<<QString::number(30000)<<
               QString::number(20000)<<QString::number(15000)<<QString::number(11025)<<QString::number(10000)<<QString::number(9000)
             <<QString::number(7500)<<QString::number(5000)<<QString::number(2500)<<QString::number(3000)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<QString::number(250)<<
               QString::number(200)<<QString::number(100);
      /*  texts<<QString::number(sr)+" Max Sampling rate"<<QString::number(sr/2)+" 1/2 of max Sampling rate"<<QString::number(sr/3)+" 1/3 of max Sampling rate"<<
                QString::number(sr/4)+" 1/4 of max Sampling rate"<<QString::number(sr/5)+" 1/5 of max Sampling rate"<<QString::number(sr/6)+" 1/6 of max Sampling rate"<<
                QString::number(sr/7)+" 1/7 of max Sampling rate"<<QString::number(sr/8)+" 1/8 of max Sampling rate"<<QString::number(sr/9)+" 1/9 of max Sampling rate"<<
                QString::number(sr/10)+" 1/10 of max Sampling rate"<<QString::number(sr/20)+" 1/20 of max Sampling rate";*/
    }
    _FilterWidget->texts.clear();
    _FilterWidget->texts = texts;
    _FilterWidget->getCutFreq()->clear();
    _FilterWidget->getNumOfOrder()->setText("2");
    _FilterWidget->getCutFreq()->addItems(_FilterWidget->texts);
}

float MainWindow::getMax(float *a,int num)
{
    float max=a[0];
    for(int i=1;i<num;i++)
        if(max<a[i])
            max=a[i];
    return max+0.1*max;
}

void MainWindow::plot2Freq()
{
    if(show2Spec->isChecked())
    {
        conFirst->setDisabled(false);
        disFirst->setDisabled(false);
        conFirst->setChecked(true);
        QVector<float> tmpw = pointer2Qvec(_data,num);
        QVector<float> tmps = utilities->getAmplitude(tmpw);
        _DisSpec->getSpecWidget()->setDataFor2Freq(tmps);
        _DisSpec->getSpecWidget()->conDis(true);
        onlyCon();
    }
    if(!show2Spec->isChecked())
    {
        conFirst->setDisabled(true);
        disFirst->setDisabled(true);
        conFirst->setChecked(false);
        disFirst->setChecked(false);
    }
}

void MainWindow::onlyCon()
{
    disFirst->setChecked(false);
    conFirst->setChecked(true);
}
void MainWindow::onlyDis()
{
    disFirst->setChecked(true);
    conFirst->setChecked(false);
}

void MainWindow::showFilter(bool i)
{
    if(i)
    {
        setFilterDataToWidgets();
        _selFilter->setChecked(true);
    }
    else
    {
        _FilterWidget->hide();
        QVector<float> tmp = pointer2Qvec(_data,num);
        _FilterWidget->getWaveWidget()->setData(tmp);
        if(_sampleData.upSampleFactor==1 && _sampleData.downSampleFactor ==1)
        {
            _sampleData.sampleData = _data;
        }
        else
        {
            _sampleData.sampleData = utilities->getSampleData(true);
        }
        _sampleFileName = QDir::tempPath()+"/tmp1.wav";
        ria->writeToWave(_sampleData.sampleData,_sampleFileName.toLatin1().data(),sr*_sampleData.upSampleFactor/_sampleData.downSampleFactor,_sampleData.currentNum);

        QVector<float> dpt;
        float* tmpd = _sampleData.sampleData;
        for( int i = 0; i < _sampleData.currentNum; i++ )
            dpt.push_back(tmpd[i]);
        _SampledWave->getWaveWidget()->setData(tmp);
        QVector<float> tmps = utilities->getAmplitude(dpt);
        _DisSpec->getSpecWidget()->setData(tmps);
        _DisSpec->getSpecWidget()->setRatio((double)_sampleData.upSampleFactor/_sampleData.downSampleFactor);
        _selFilter->setChecked(false);
    }
}


void MainWindow::nonIntSr(int s)
{
    if(s<0)
        s=0;
    int nums=_samplingRates[s];
    if(nums==sr)
    {
        _sampleData.upSampleFactor=1;
        _sampleData.downSampleFactor=1;
    }
    else
    {
        int Gcd = gcd(nums,sr);
        _sampleData.upSampleFactor= nums/Gcd;
        _sampleData.downSampleFactor = sr/Gcd;
    }
    _sampleData.currentNum=num*_sampleData.upSampleFactor/_sampleData.downSampleFactor;
    utilities->setSampleFactor(_sampleData.upSampleFactor,_sampleData.downSampleFactor,_sampleData.currentNum);
    _sampleData.sampleData = utilities->getSampleData(!_selFilter->isChecked());
    _sampleFileName = QDir::tempPath()+"/tmp1.wav";
    ria->writeToWave(_sampleData.sampleData,_sampleFileName.toLatin1().data(),nums,_sampleData.currentNum);
    QVector<float> dpt;
    float* tmpd = _sampleData.sampleData;
    for( int i = 0; i < _sampleData.currentNum; i++ )
        dpt.push_back(tmpd[i]);
    int step = _sampleData.downSampleFactor/_sampleData.upSampleFactor;
    _SampledWave->getWaveWidget()->setOffset(step);
    QVector<float> tmps = utilities->getAmplitude(dpt);
    _DisSpec->getSpecWidget()->setData(tmps);
    _DisSpec->getSpecWidget()->setRatio((double)_sampleData.upSampleFactor/_sampleData.downSampleFactor);
    _ConSpec->getSpecWidget()->setRatio((double)_sampleData.upSampleFactor/_sampleData.downSampleFactor);
    utilities->setFactor((float)_sampleData.upSampleFactor/_sampleData.downSampleFactor);
    utilities->setNyqFreq((float)_sampleData.upSampleFactor/_sampleData.downSampleFactor/2);
}

void MainWindow::applyFilters()
{
    QVector<float> filterdata = utilities->getFilterData();
    _filteredFileName = QDir::tempPath()+"/tmp2.wav";
    ria->writeToWave(filterdata.data(),_filteredFileName.toLatin1().data(),sr,num);
    if(_sampleData.upSampleFactor==1&&_sampleData.downSampleFactor==1)
    {
        utilities->setOnlyFiltered(true);
    }
    else
    {
        utilities->setOnlyFiltered(false);
    }
    _sampleData.sampleData = utilities->getSampleData(false);
    _FilterWidget->getWaveWidget()->setData(filterdata);
    QVector<float> tmp = utilities->getAmplitude(filterdata);
    _FilterWidget->getSpecWidget()->setData(tmp);
    _sampleFileName = QDir::tempPath()+"/tmp1.wav";
    ria->writeToWave(_sampleData.sampleData,_sampleFileName.toLatin1().data(),sr*_sampleData.upSampleFactor/_sampleData.downSampleFactor,_sampleData.currentNum);
    QVector<float> dpt;
    float* tmpd = _sampleData.sampleData;
    for( int i = 0; i < _sampleData.currentNum; i++ )
        dpt.push_back(tmpd[i]);
     _SampledWave->getWaveWidget()->setData(filterdata);
    QVector<float> tmps = utilities->getAmplitude(dpt);
    _DisSpec->getSpecWidget()->setData(tmps);
    _DisSpec->getSpecWidget()->setRatio((double)_sampleData.upSampleFactor/_sampleData.downSampleFactor);
}

void MainWindow::setBits(int bits)
{
    this->bits = bits;
    utilities->setBit(bits);
    _quantizedData = utilities->getQuantize();
    QVector<float> dpt = pointer2Qvec(_quantizedData,_sampleData.currentNum);
    _SampledWave->getWaveWidget()->setData(utilities->getOrgQuan());
     _SampledWave->getWaveWidget()->update();
    QVector<float> tmps = utilities->getAmplitude(dpt);
    _DisSpec->getSpecWidget()->setData(tmps);
    _sampleFileName = QDir::tempPath()+"/tmp1.wav";
    ria->writeToWave(_quantizedData,_sampleFileName.toLatin1().data(),sr*_sampleData.upSampleFactor/_sampleData.downSampleFactor,_sampleData.currentNum);
    if(bits >= _trueBits)
    {
        SNR->setText("inf");
        _SampledWave->getWaveWidget()->update();
    }
    else
    {
        double error=  utilities->computeError();
        double snr = utilities->getSNR(error);
        _SampledWave->getWaveWidget()->setSNR(snr);
    }
}

void MainWindow::resetEverything()
{
    sampleRateSelect->setCurrentIndex(0);
    _showOnePeriod->setChecked(false);
    _showSampleRange->setChecked(false);
    show2Spec->setChecked(false);
    conFirst->setChecked(false);
    disFirst->setChecked(false);
    _selFilter->setChecked(false);
    _showAliasing->setChecked(false);
    SNR->setText("inf");
    sampleRateSelect->setEnabled(true);
    bitBox->setEnabled(true);
}

void MainWindow::displayAliasing(bool i)
{
    if(i)
    {
        QVector<float> tmpw = pointer2Qvec(_data,num);
        QVector<float> tmps = utilities->getAmplitude(tmpw);
        _DisSpec->getSpecWidget()->setDataForAliasing(tmps);
    }
    _DisSpec->getSpecWidget()->showAliasing(i);
    sampleRateSelect->setEnabled(!i);
    bitBox->setEnabled(!i);
}

void MainWindow::enableNoiseSelect(bool i)
{
    _selectNoise->setEnabled(i);
    if(i)
    {
        noiseAddfunc(_selectNoise->currentIndex());
    }
    if(!i)
    {
        for(int i =0; i < num; i++)
            _data[i] =_orgData[i];
        utilities->updateUtilites(_data,num);
        _orgFileName=QDir::tempPath()+"/tmp.wav";
        ria->writeToWave(_data,_orgFileName.toLatin1().data(),sr,num);
        QVector<float> tmpw = pointer2Qvec(_data,num);
        QVector<float> tmps = utilities->getAmplitude(tmpw);
        _ConSpec->getSpecWidget()->setData(tmps);
     //   _ConSpec->getSpecWidget()->updateMax();
        _SampledWave->getWaveWidget()->setData(tmpw);
        _OrgWave->getWaveWidget()->setData(tmpw);
        _OrgWave->getWaveWidget()->updateMax();
        _SampledWave->getWaveWidget()->updateMax();
        nonIntSr(sampleRateSelect->currentIndex());
        //_DisSpec->getSpecWidget()->updateMax();
    }
}

void MainWindow::noiseAddfunc(int index)
{
    float freq = 0;
    if(index == -1 || index == 0)
    {
       freq = 0.25*M_PI;
    }
    else if(index == 1)
    {
        freq = 0.5 *M_PI;
    }
    else if(index == 2)
    {
        freq = 0.75 * M_PI;
    }
    if(loadingFailed)
    {
        for(int i =0; i < num; i++)
            _data[i] =(_orgData[i] + (float)sin(freq*i)/10.0)/1.1;
    }
    else
    {
        for(int i =0; i < num; i++)
            _data[i] =(_orgData[i] + (float)sin(freq*i)/10.0)/1.1;
    }
    utilities->updateUtilites(_data,num);
    _orgFileName=QDir::tempPath()+"/tmp.wav";
    _sampleFileName = _orgFileName;

    ria->writeToWave(_data,_orgFileName.toLatin1().data(),sr,num);
     nonIntSr(sampleRateSelect->currentIndex());
     QVector<float> tmpw = pointer2Qvec(_data,num);
     QVector<float> tmps = utilities->getAmplitude(tmpw);
     _ConSpec->getSpecWidget()->setData(tmps);
     _SampledWave->getWaveWidget()->setData(tmpw);
     _OrgWave->getWaveWidget()->setData(tmpw);
     nonIntSr(sampleRateSelect->currentIndex());
}

QVector<float> MainWindow::pointer2Qvec(float *data,int size)
{
    QVector<float> tmp;
    for( int i = 0; i < size; i++ )
        tmp.push_back(data[i]);
    return tmp;
}

void MainWindow::butterOrCheby(int index)
{
    if((index==-1)||(index==0))
        utilities->butterOrCheby(true);
    else utilities->butterOrCheby(false);
}

void MainWindow::setOrder(QString order)
{
    int ord=order.toInt();
    if(ord > 15)
    {
       // if(filterData!=NULL)
           // free(filterData);
       // filterData=(float*)malloc(number*sizeof(float));
       // for(int i = 0;i < number; i++)
           // filterData[i] = data[i];
        emit OrderTooHigh(true);
        return;
    }
    utilities->setOrder(ord);
}

void MainWindow::setNyqFreq()
{
    utilities->setFactor(utilities->getNyq());
    applyFilters();
}

void MainWindow::setRipple(QString ripple)
{
    float rip=ripple.toFloat();
    if(rip <= 0)
    {
      //  if(filterData!=NULL)
            //free(filterData);
      //  filterData=(float*)malloc(number*sizeof(float));
        emit RippleNotValid(true);
        return;
    }
    else emit RippleNotValid(false);
    utilities->setRipple(rip);
}

void MainWindow::setFilterDataToWidgets()
{
    QVector<float> filterdata = utilities->getFilterData();
    _FilterWidget->getWaveWidget()->setData(filterdata);
    _FilterWidget->show();
    QVector<float> amt = utilities->getAmplitude(filterdata);
    _FilterWidget->getSpecWidget()->setData(amt);
    _sampleData.sampleData = utilities->getSampleData(false);
    _sampleFileName = QDir::tempPath()+"/tmp1.wav";
    ria->writeToWave(_sampleData.sampleData,_sampleFileName.toLatin1().data(),sr*_sampleData.upSampleFactor/_sampleData.downSampleFactor,_sampleData.currentNum);
    QVector<float> dpt;
    float* tmpd = _sampleData.sampleData;
    for( int i = 0; i < _sampleData.currentNum; i++ )
        dpt.push_back(tmpd[i]);
     _SampledWave->getWaveWidget()->setData(filterdata);
    QVector<float> tmps = utilities->getAmplitude(dpt);
    _DisSpec->getSpecWidget()->setData(tmps);
    _DisSpec->getSpecWidget()->setRatio((double)_sampleData.upSampleFactor/_sampleData.downSampleFactor);
}

void MainWindow::setFactor(QString s)
{
    float factor=s.toFloat()/(float)sr;
    utilities->setFactor(factor);
}
