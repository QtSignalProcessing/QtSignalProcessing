#include "mainwindow.h"

#include "plotwidget.h"
#include "utilities.h"
#include "audiohandle.h"
#include "plotfilter.h"
#include "glsw.h"
#include "glospectrum.h"

#include <QMouseEvent>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMenuBar>
#include <QFileDialog>
#include <QApplication>
#include <QToolButton>
#include <QScrollBar>
#include <QGridLayout>
#include <QLabel>

#if QT_VERSION < 0x050000
#include <phonon/MediaObject>
#else
#include <QMediaPlayer>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),buf1(NULL),sampleData(NULL),qData(NULL),bits(0),tmpData(NULL),ifShowAlias(false),
      alreadyFiltered(false),_trueBits(0),_selFilter(NULL),_showAliasing(NULL),_showSampleRange(NULL),_showOnePeriod(NULL),loadingFailed(false),_orgData(NULL),
      _orgFileName(),_sampleFileName(),_filteredFileName()
{
    createActions();
    createMenus();
    ria = new AudioHandle(QString("helloDSP.wav").toStdString().c_str());
    _orgFileName = "helloDSP.wav";
    main=new QWidget(this);
    this->setCentralWidget(main);
    createWidget(main);
    this->setWindowTitle("DSP Demo - University of Konstanz");
    this->show();
    if(loadingFailed)
    {
        QMessageBox error;
        error.setText("Default audio file open failed, and sin signal with noise will be shown");
        error.exec();
    }
}

void MainWindow::initializeVar()
{
    ripple=0.5;
    Butter=true;
    order=2;
    appFilter=false;
    L=1;
    down=1;
    f = ria->getFrameNum();
    sr=ria->getSamplerate();
    c=ria->getChannel();
    num_items = f*c;
    appFilter = false;
    time=(float)num_items/sr;
    if(buf1!=NULL)
        free(buf1);
    buf1=ria->getData(num_items,num);
    if(buf1)
    {
        currentNum=num;
        loadingFailed = false;
        if(_orgData!=NULL)
            free(_orgData);
        _orgData = (float*)malloc(num*sizeof(float));
        for(int i =0; i < currentNum; i++)
            _orgData[i] =buf1[i] ;
    }
    else
    {

        num = 20000;
        currentNum=num;
        buf1 = (float*)malloc(sizeof(float)*num);
        sr = 11025;
        for(int i = 0; i< 20000;i++)
            buf1[i] = (sin((double)i/1000.0)+sin((double)i))/2.0;
        _orgData = (float*)malloc(num*sizeof(float));
        for(int i =0; i < currentNum; i++)
            _orgData[i] =buf1[i] ;
        loadingFailed = true;
        _orgFileName=QDir::tempPath()+"/tmp.wav";
        ria->writeToWave(buf1,_orgFileName.toLatin1().data(),sr,num);
    }
}

void MainWindow::createWidget(QWidget *main)
{
     initializeVar();
    //create the widget Plot Sampled waveform
    utilities=new Utilities(buf1,num);
    _SampledWave = new PlotWidget(buf1,num,time,true,main);
    //create the widget to plot the oroginal waveform
    _OrgWave = new PlotWidget(buf1,num,time,false,main);
    discrete = false;
    //create the widget for the continuous-time spectrum
    _ConSpec = new PlotWidget(buf1,num,discrete,sr,main);
    //discrete-time spectrum
    _DisSpec=new PlotWidget(buf1,num,true,sr,main);
    _OrgWave->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _ConSpec->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _SampledWave->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _DisSpec->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(_OrgWave->hReset,SIGNAL(clicked()),this,SLOT(resetW1()));
    connect(_SampledWave->hReset,SIGNAL(clicked()),this,SLOT(resetW1()));
    connect(_OrgWave->hSc,SIGNAL(valueChanged(int)),this,SLOT(bindWaveScroll(int)));
    connect(_SampledWave->hSc,SIGNAL(valueChanged(int)),this,SLOT(bindWaveScroll(int)));
    connect(_ConSpec->hReset,SIGNAL(clicked()),this,SLOT(resetSp()));
    connect(_DisSpec->hReset,SIGNAL(clicked()),this,SLOT(resetSp()));
    connect(_ConSpec->hSc,SIGNAL(valueChanged(int)),this,SLOT(bindSpecScroll(int)));
    connect(_DisSpec->hSc,SIGNAL(valueChanged(int)),this,SLOT(bindSpecScroll(int)));
    connect(_OrgWave->vReset,SIGNAL(clicked()),this,SLOT(vresetW1()));
    connect(_SampledWave->vReset,SIGNAL(clicked()),this,SLOT(vresetW1()));
    connect(_OrgWave->vSc,SIGNAL(valueChanged(int)),this,SLOT(vbindWaveScroll(int)));
    connect(_SampledWave->vSc,SIGNAL(valueChanged(int)),this,SLOT(vbindWaveScroll(int)));
    connect(_ConSpec->vReset,SIGNAL(clicked()),this,SLOT(vresetSp()));
    connect(_DisSpec->vReset,SIGNAL(clicked()),this,SLOT(vresetSp()));
    connect(_ConSpec->vSc,SIGNAL(valueChanged(int)),this,SLOT(vbindSpecScroll(int)));
    connect(_DisSpec->vSc,SIGNAL(valueChanged(int)),this,SLOT(vbindSpecScroll(int)));
    connect(_SampledWave->hSc,SIGNAL(valueChanged(int)),_OrgWave->getGlwidget(),SLOT(intToDouble(int)));
    connect(_OrgWave->hSc,SIGNAL(valueChanged(int)),_SampledWave->getGlwidget(),SLOT(intToDouble(int)));
    connect(_SampledWave->vSc,SIGNAL(valueChanged(int)),_OrgWave->getGlwidget(),SLOT(setVshift(int)));
    connect(_OrgWave->vSc,SIGNAL(valueChanged(int)),_SampledWave->getGlwidget(),SLOT(setVshift(int)));
    connect(_OrgWave->vReset,SIGNAL(clicked()),_SampledWave->getGlwidget(),SLOT(resetV()));
    connect(_SampledWave->vReset,SIGNAL(clicked()),_OrgWave->getGlwidget(),SLOT(resetV()));
    connect(_OrgWave->hReset,SIGNAL(clicked()),_SampledWave->getGlwidget(),SLOT(resetH()));
    connect(_SampledWave->hReset,SIGNAL(clicked()),_OrgWave->getGlwidget(),SLOT(resetH()));
    connect(_OrgWave->vPlus,SIGNAL(clicked()),_SampledWave->getGlwidget(),SLOT(vIncrease()));
    connect(_SampledWave->vPlus,SIGNAL(clicked()),_OrgWave->getGlwidget(),SLOT(vIncrease()));
    connect(_OrgWave->vMinus,SIGNAL(clicked()),_SampledWave->getGlwidget(),SLOT(vDecrease()));
    connect(_SampledWave->vMinus,SIGNAL(clicked()),_OrgWave->getGlwidget(),SLOT(vDecrease()));
    connect(_OrgWave->hPlus,SIGNAL(clicked()),_SampledWave->getGlwidget(),SLOT(hIncrease()));
    connect(_SampledWave->hPlus,SIGNAL(clicked()),_OrgWave->getGlwidget(),SLOT(hIncrease()));
    connect(_OrgWave->hMinus,SIGNAL(clicked()),_SampledWave->getGlwidget(),SLOT(hDecrease()));
    connect(_SampledWave->hMinus,SIGNAL(clicked()),_OrgWave->getGlwidget(),SLOT(hDecrease()));
    connect(_ConSpec->hSc,SIGNAL(valueChanged(int)),_DisSpec->getWidget(),SLOT(intToDouble(int)));
    connect(_DisSpec->hSc,SIGNAL(valueChanged(int)),_ConSpec->getWidget(),SLOT(intToDouble(int)));
    connect(_ConSpec->vSc,SIGNAL(valueChanged(int)),_DisSpec->getWidget(),SLOT(setVshift(int)));
    connect(_DisSpec->vSc,SIGNAL(valueChanged(int)),_ConSpec->getWidget(),SLOT(setVshift(int)));
    connect(_ConSpec->vPlus,SIGNAL(clicked()),_DisSpec->getWidget(),SLOT(vIncrease()));
    connect(_DisSpec->vPlus,SIGNAL(clicked()),_ConSpec->getWidget(),SLOT(vIncrease()));
    connect(_ConSpec->vMinus,SIGNAL(clicked()),_DisSpec->getWidget(),SLOT(vDecrease()));
    connect(_DisSpec->vMinus,SIGNAL(clicked()),_ConSpec->getWidget(),SLOT(vDecrease()));
    connect(_ConSpec->hPlus,SIGNAL(clicked()),_DisSpec->getWidget(),SLOT(hIncrease()));
    connect(_DisSpec->hPlus,SIGNAL(clicked()),_ConSpec->getWidget(),SLOT(hIncrease()));
    connect(_ConSpec->hMinus,SIGNAL(clicked()),_DisSpec->getWidget(),SLOT(hDecrease()));
    connect(_DisSpec->hMinus,SIGNAL(clicked()),_ConSpec->getWidget(),SLOT(hDecrease()));
    connect(_ConSpec->vReset,SIGNAL(clicked()),_DisSpec->getWidget(),SLOT(resetV()));
    connect(_DisSpec->vReset,SIGNAL(clicked()),_ConSpec->getWidget(),SLOT(resetV()));
    connect(_ConSpec->hReset,SIGNAL(clicked()),_DisSpec->getWidget(),SLOT(resetH()));
    connect(_DisSpec->hReset,SIGNAL(clicked()),_ConSpec->getWidget(),SLOT(resetH()));
    QGridLayout *ConpLayout = new QGridLayout(main);
    ConpLayout->addWidget(_OrgWave,0,0);
    ConpLayout->addWidget(_ConSpec,0,2);
    ConpLayout->addWidget(_SampledWave,2,0);
    ConpLayout->addWidget(_DisSpec,2,2);
    connect(_DisSpec->getWidget(),SIGNAL(TooManyPeriods(bool)),this,SLOT(frozenScaling(bool)));
    QToolButton *playB=new QToolButton(main);
    playB->setText("play");
    playB->connect(playB,SIGNAL(clicked()),this,SLOT(play()));
    ConpLayout->addWidget(playB,1,0);
    QToolButton *play1=new QToolButton(main);
    play1->setText("play");
    play1->connect(play1,SIGNAL(clicked()),this,SLOT(playSample()));
    QCheckBox* stickOn = new QCheckBox("Stick on",this);
    connect(stickOn,SIGNAL(toggled(bool)),_SampledWave->getGlwidget(),SLOT(setStickOn(bool)));
    QHBoxLayout* bottomLeft = new QHBoxLayout;
    bottomLeft->addWidget(play1);
    bottomLeft->addWidget(stickOn);
    ConpLayout->addLayout(bottomLeft,3,0);
    QWidget* rightWidget=new QWidget(main);
    QVBoxLayout *vLayout=new QVBoxLayout;
    //quantization
    if(loadingFailed)
    {
        _trueBits = 8;
    }
    else
    {
        _trueBits = utilities->computeTrueBits();
    }
    bitBox=new QSpinBox(main);
    bitBox->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    bitBox->setValue(_trueBits);
    bitBox->setMaximum(16);
    bitBox->setMinimum(1);
    //connect to glsw for plotting the quantized waveform and glospectrum for computing the FFT and plotting
    connect(bitBox,SIGNAL(valueChanged(int)),this,SLOT(setBits(int)));
    connect(bitBox,SIGNAL(valueChanged(int)),_SampledWave->getGlwidget(),SLOT(setBits(int)));
    connect(bitBox,SIGNAL(valueChanged(int)),_DisSpec->getWidget(),SLOT(setBits(int)));
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
    texts<<QString::number(sr)<<QString::number(10000)<<QString::number(5000)<<
           QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
           QString::number(200)<<QString::number(100);
    }
    if(sr == 44100)
    {
        texts<<QString::number(sr)<<QString::number(40000)<<QString::number(30000)<<
               QString::number(20000)<<QString::number(10000)<<QString::number(5000)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
               QString::number(200)<<QString::number(100);
    }
    sampleRateSelect->addItems(texts);
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),this,SLOT(nonIntSr(QString)));
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_SampledWave->getGlwidget(),SLOT(nonIntSr(QString)));
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_DisSpec->getWidget(),SLOT(nonIntSr(QString)));
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_ConSpec->getWidget(),SLOT(setFactor(QString)));
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
    connect(_showOnePeriod,SIGNAL(clicked(bool)),_DisSpec->getWidget(),SLOT(show1Period(bool)));
    connect(_showSampleRange,SIGNAL(stateChanged(int)),_ConSpec->getWidget(),SLOT(showSampleRange(int)));
    connect(_showSampleRange,SIGNAL(stateChanged(int)),_DisSpec->getWidget(),SLOT(showSampleRange(int)));
    h1->addWidget(_showSampleRange);
    h1->addWidget(_showOnePeriod);
    QWidget* samplerange=new QWidget(main);
    samplerange->setLayout(h1);
    QGridLayout* v1=new QGridLayout;
    QLabel* filterLabel=new QLabel("Apply anti-aliasing filter",this);
    _addNoise = new QCheckBox("Add sin noise to signal",this);
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
    connect(show2Spec,SIGNAL(clicked(bool)),_DisSpec->getWidget(),SLOT(show2F(bool)));
    conFirst=new QCheckBox("Cont. first",main);
    disFirst=new QCheckBox("Discrete first",main);
    conFirst->setDisabled(true);
    disFirst->setDisabled(true);
    connect(conFirst,SIGNAL(clicked()),this,SLOT(onlyCon()));
    connect(conFirst,SIGNAL(clicked(bool)),_DisSpec->getWidget(),SLOT(conDis()));
    connect(disFirst,SIGNAL(clicked()),this,SLOT(onlyDis()));
    connect(disFirst,SIGNAL(clicked(bool)),_DisSpec->getWidget(),SLOT(disCon()));
    QHBoxLayout* h4=new QHBoxLayout;
    h4->addWidget(conFirst);
    h4->addWidget(disFirst);
    QWidget* _show2Spec=new QWidget(main);
    _show2Spec->setLayout(h4);
    vLayout->addWidget(show2Spec);
    vLayout->addWidget(_show2Spec);
    connect(_SampledWave->getGlwidget(),SIGNAL(newSNR(double)),SNR,SLOT(setNum(double)));
    rightWidget->setLayout(vLayout);
    ConpLayout->addWidget(rightWidget,2,3);
    _FilterWidget=new plotFilter(buf1,num,false,sr,0);
    _FilterWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_FilterWidget->getWidget(),SLOT(setNyqFreq(QString)));
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_FilterWidget->getWidget(),SLOT(setFactor(QString)));
    connect(_FilterWidget->apply,SIGNAL(clicked()),this,SLOT(applyFilters()));
    connect(_FilterWidget->reset,SIGNAL(clicked()),this,SLOT(applyFilters()));
    connect(sampleRateSelect,SIGNAL(currentIndexChanged(QString)),_FilterWidget,SLOT(setComboText(QString)));
    connect(_FilterWidget->play,SIGNAL(clicked()),this,SLOT(playfiltered()));
    connect(_FilterWidget,SIGNAL(closed(bool)),this,SLOT(showFilter(bool)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::play()
{
#if QT_VERSION < 0x050000
  Phonon::MediaObject* player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_orgFileName));
#else
  QMediaPlayer* player = new QMediaPlayer();
  player->setMedia(QUrl::fromLocalFile(_orgFileName));
#endif
  player->play();
}

void MainWindow::playSample(){
#if QT_VERSION < 0x050000
  Phonon::MediaObject* player;
  if (_sampleFileName == NULL)
  {
    player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_orgFileName));
  }
  else
  {
    player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_sampleFileName));
  }
#else
  QMediaPlayer* player = new QMediaPlayer();
  if (_sampleFileName == NULL)
  {
      player->setMedia(QUrl::fromLocalFile(_orgFileName));
  }
  else
  {
        player->setMedia(QUrl::fromLocalFile(_sampleFileName));
  }
#endif
  player->play();
}

void MainWindow::playfiltered()
{
  if(_filteredFileName!=NULL)
  {
#if QT_VERSION < 0x050000
  Phonon::MediaObject* player = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(_filteredFileName));
#else
  QMediaPlayer* player = new QMediaPlayer();
  player->setMedia(QUrl::fromLocalFile(_filteredFileName));
#endif
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
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

//load in the selected audio file(need to add the filtering of the file types)
void MainWindow::loadFile(const QString &fileName)
{
    char *s = fileName.toLatin1().data();
    if(ria!=NULL)
        delete ria;
    ria = new AudioHandle(s);
    initializeVar();
    QStringList texts;
   if(sr==11025)
    {
        texts<<QString::number(sr)<<QString::number(10000)<<QString::number(5000)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
               QString::number(200)<<QString::number(100);
    }
    else if(sr == 44100)
    {
        texts<<QString::number(sr)<<QString::number(40000)<<QString::number(30000)<<
               QString::number(20000)<<QString::number(10000)<<QString::number(5000)<<
               QString::number(2000)<<QString::number(1000)<<QString::number(500)<<
               QString::number(200)<<QString::number(100);
    }
   updateFilter();
   sampleRateSelect->blockSignals(true);
   sampleRateSelect->clear();
   sampleRateSelect->addItems(texts);
   sampleRateSelect->blockSignals(false);
   loadOrgwave();
   loadSample();
   loadCSpec();
   loadDSpec();
   utilities->updateUtilites(buf1,num);
   if(loadingFailed)
   {
       _trueBits = 8;
   }
   else
   {
    _trueBits = utilities->computeTrueBits();
   }
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
void MainWindow::loadOrgwave()
{
    _OrgWave->getGlwidget()->resetDataAndOther(buf1,num,time,getMax(buf1,num));
    _OrgWave->getGlwidget()->updateLabel(time,0);
    _OrgWave->getGlwidget()->resetCommon();
    _OrgWave->getGlwidget()->resetV();
    _OrgWave->getGlwidget()->resetH();
}

void MainWindow::loadSample()
{
    _SampledWave->getGlwidget()->setSampleFactors(0,0);
    _SampledWave->getGlwidget()->sampleOrNot=true;
     _SampledWave->getGlwidget()->updateLabel(time,0);
    _SampledWave->getGlwidget()->resetDataAndOther(buf1,num,time,getMax(buf1,num));
    _SampledWave->getGlwidget()->resetCommon();
    _SampledWave->getGlwidget()->resetV();
    _SampledWave->getGlwidget()->resetH();
}


void MainWindow::loadCSpec()
{
    _ConSpec->getWidget()->resetCommon();
    _ConSpec->getWidget()->discrete = false;
    _ConSpec->getWidget()->resetSpec(buf1,num,sr);
    _ConSpec->getWidget()->resetV();
    _ConSpec->getWidget()->resetH();
}

void MainWindow::loadDSpec()
{
  _DisSpec->getWidget()->resetCommon();
  _DisSpec->getWidget()->discrete = true;
  _DisSpec->getWidget()->resetSpec(buf1,num,sr);
  _DisSpec->getWidget()->resetV();
  _DisSpec->getWidget()->resetH();
}

void MainWindow::updateFilter()
{
  _FilterWidget->getWidget()->resetCommon();
  _FilterWidget->getWidget()->resetSpec(buf1,num,sr);
  _FilterWidget->getGLWidget()->resetCommon();
   _FilterWidget->getGLWidget()->resetDataAndOther(buf1,num,time,getMax(buf1,num));
   _FilterWidget->getGLWidget()->updateLabel(time,0);
  QStringList texts;
  if(sr==11025)
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
  }
  _FilterWidget->texts.clear();
  _FilterWidget->texts = texts;
  _FilterWidget->cutFreqSel->clear();
  _FilterWidget->numOfOrder->setText("2");
  _FilterWidget->cutFreqSel->addItems(_FilterWidget->texts);
  _FilterWidget->getWidget()->Filter();
  _FilterWidget->setActualFreq();
  _FilterWidget->getWidget()->updateGL();
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
    alreadyFiltered = true;
    appFilter=true;
    _FilterWidget->getWidget()->showFilter=true;
    _FilterWidget->getWidget()->Filter();
    _FilterWidget->show();
    getFilterdata();
    _FilterWidget->getGLWidget()->plotData = filterData;
    utilities->setFilterData(filterData);
    sampleData = utilities->getSampleData(false);
    _SampledWave->getGlwidget()->setL(L,currentNum,down,true);
    _SampledWave->getGlwidget()->plotData=sampleData;
    _sampleFileName = QDir::tempPath()+"/tmp1.wav";
    ria->writeToWave(sampleData,_sampleFileName.toLatin1().data(),sr*L/down,currentNum);
    _SampledWave->getGlwidget()->updateGL();
    _DisSpec->getWidget()->plotData=sampleData;
    _DisSpec->getWidget()->setL(L,currentNum,down,true);
    _DisSpec->getWidget()->updateGL();
    _selFilter->setChecked(true);
  }
  else
  {
    _FilterWidget->hide();
    appFilter=false;
    _FilterWidget->getGLWidget()->plotData = buf1;
    if(L==1 && down ==1)
    {
      sampleData = buf1;
    }
    else
    {
      sampleData = utilities->getSampleData(true);
    }
     _SampledWave->getGlwidget()->setL(L,currentNum,down,false);
      _sampleFileName = QDir::tempPath()+"/tmp1.wav";
    ria->writeToWave(sampleData,_sampleFileName.toLatin1().data(),sr*L/down,currentNum);
    _SampledWave->getGlwidget()->plotData=sampleData;
    _SampledWave->getGlwidget()->updateGL();
    _DisSpec->getWidget()->plotData=sampleData;
    _DisSpec->getWidget()->setL(L,currentNum,down,false);
    _DisSpec->getWidget()->updateGL();
    _selFilter->setChecked(false);
  }
}

int MainWindow::gcd(int v1,int v2)
{
  while(v2)
  {
    int temp=v2;
    v2=v1%v2;
    v1=temp;
  }
  return v1;
}

void MainWindow::nonIntSr(QString s)
{
  int nums=s.toInt();
  if(nums==0||nums==-1||nums==sr)
  {
    L=1;
    down=1;
  }
  else
  {
    int Gcd=gcd(nums,sr);
    L=nums/Gcd;
    down=sr/Gcd;
  }
  currentNum=num*L/down;
  utilities->setSampleFactor(L,down,currentNum);
  sampleData = utilities->getSampleData(!appFilter);
  _SampledWave->getGlwidget()->setL(L,currentNum,down,false);
  _SampledWave->getGlwidget()->plotData=sampleData;
  _sampleFileName = QDir::tempPath()+"/tmp1.wav";
  ria->writeToWave(sampleData,_sampleFileName.toLatin1().data(),nums,currentNum);
  _DisSpec->getWidget()->plotData=sampleData;
  _DisSpec->getWidget()->setL(L,currentNum,down,false);
}

void MainWindow::getFilterdata()
{
  this->filterData=_FilterWidget->getWidget()->filterData;
}

void MainWindow::applyFilters()
{
  getFilterdata();
  _filteredFileName = QDir::tempPath()+"/tmp2.wav";
  ria->writeToWave(filterData,_filteredFileName.toLatin1().data(),sr,num);
  if(L==1&&down==1)
  {
      utilities->setOnlyFiltered(true);
  }
  else
  {
      utilities->setOnlyFiltered(false);
  }
  utilities->setFilterData(filterData);
  sampleData = utilities->getSampleData(false);
  _SampledWave->getGlwidget()->plotData=sampleData;
  _FilterWidget->getGLWidget()->updateGL();
  _SampledWave->getGlwidget()->setL(L,currentNum,down,true);
  _sampleFileName = QDir::tempPath()+"/tmp1.wav";
  ria->writeToWave(sampleData,_sampleFileName.toLatin1().data(),sr*L/down,currentNum);
  _SampledWave->getGlwidget()->updateGL();
  _DisSpec->getWidget()->plotData=sampleData;
  _DisSpec->getWidget()->setL(L,currentNum,down,true);
  _DisSpec->getWidget()->updateGL();
}

void MainWindow::resetW1()
{
  _OrgWave->hSc->setSliderPosition(0);
  _SampledWave->hSc->setSliderPosition(0);
}

void MainWindow::bindWaveScroll(int s)
{
 _SampledWave->hSc->setSliderPosition(s);
 _OrgWave->hSc->setSliderPosition(s);
}

void MainWindow::resetSp()
{
  _ConSpec->hSc->setSliderPosition(0);
  _DisSpec->hSc->setSliderPosition(0);
}

void MainWindow::bindSpecScroll(int s)
{
  _ConSpec->hSc->setSliderPosition(s);
  _DisSpec->hSc->setSliderPosition(s);
}

void MainWindow::vresetW1()
{
  _OrgWave->vSc->setSliderPosition(0);
  _SampledWave->vSc->setSliderPosition(0);
}

void MainWindow::vbindWaveScroll(int s)
{
  _OrgWave->vSc->setSliderPosition(s);
  _SampledWave->vSc->setSliderPosition(s);
}

void MainWindow::vresetSp()
{
  _ConSpec->hSc->setSliderPosition(0);
  _DisSpec->hSc->setSliderPosition(0);
}

void MainWindow::vbindSpecScroll(int s)
{
  _ConSpec->vSc->setSliderPosition(s);
  _DisSpec->vSc->setSliderPosition(s);
}

void MainWindow::setBits(int bits)
{
  this->bits = bits;
  utilities->setBit(bits);
  qData = utilities->getQuantize();
  _SampledWave->getGlwidget()->plotData = qData;
  _DisSpec->getWidget()->plotData = qData;
  _sampleFileName = QDir::tempPath()+"/tmp1.wav";
  ria->writeToWave(qData,_sampleFileName.toLatin1().data(),sr*L/down,currentNum);
  if(bits >= _trueBits)
  {
    SNR->setText("inf");
    _SampledWave->getGlwidget()->updateGL();
  }
  else
  {
    double error=  utilities->computeError();
    double snr = utilities->getSNR(error);
    _SampledWave->getGlwidget()->setSNR(snr);
    _SampledWave->getGlwidget()->updateGL();
  }
}

void MainWindow::resetEverything()
{
  vresetSp();
  vresetW1();
  resetSp();
  resetW1();
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

void MainWindow::frozenScaling(bool i)
{
  _ConSpec->getWidget()->setBanned(i);
  _ConSpec->hSc->setEnabled(!i);
  _ConSpec->vSc->setEnabled(!i);
  _ConSpec->hPlus->setEnabled(!i);
  _ConSpec->vPlus->setEnabled(!i);
  _ConSpec->hMinus->setEnabled(!i);
  _ConSpec->vMinus->setEnabled(!i);
}

void MainWindow::displayAliasing(bool i)
{
  _DisSpec->getWidget()->setShowAliasing(i);
  sampleRateSelect->setEnabled(!i);
  bitBox->setEnabled(!i);
  resetSp();
  vresetSp();
}

void MainWindow::wheelEvent(QWheelEvent *e)
{
  if(e->pos().x() > _OrgWave->pos().x() && e->pos().x()<(_OrgWave->pos().x()+_OrgWave->width()))
  {
    if(e->delta()>0)
    {
      double p = e->pos().x();
      p = (float)p/_OrgWave->hSc->width() * 100 -50;
      _OrgWave->getGlwidget()->setHshift(p);
      _SampledWave->getGlwidget()->setHshift(p);
      _OrgWave->getGlwidget()->hIncrease();
      _SampledWave->getGlwidget()->hIncrease();
      qDebug()<<_ConSpec->pos();
    }
    else
    {
      double p = e->pos().x();
      p = (float)p/_OrgWave->hSc->width() * 100 -50;
      _OrgWave->getGlwidget()->setHshift(p);
      _SampledWave->getGlwidget()->setHshift(p);
      _OrgWave->getGlwidget()->hDecrease();
      _SampledWave->getGlwidget()->hDecrease();
    }
  }
  else if(e->pos().x() > _ConSpec->pos().x() && e->pos().x()<(_ConSpec->width()+_ConSpec->pos().x()))
  {
    if(e->delta()>0)
    {
      _DisSpec->getWidget()->hIncrease();
      _ConSpec->getWidget()->hIncrease();
    }
    else
    {
      _DisSpec->getWidget()->hDecrease();
      _ConSpec->getWidget()->hDecrease();
    }
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
  if(event->buttons() & Qt::LeftButton)
  {
    if(event->pos().x() > _OrgWave->pos().x() && event->pos().x()<(_OrgWave->pos().x()+_OrgWave->width()))
    {
      double p = event->pos().x();
      p = (float)p/_OrgWave->width() * 100 -50;
      _OrgWave->getGlwidget()->setHshift(p);
      _SampledWave->getGlwidget()->setHshift(p);
      _OrgWave->hSc->blockSignals(true);
      _SampledWave->hSc->blockSignals(true);
      bindWaveScroll(p);
      _OrgWave->hSc->blockSignals(false);
      _SampledWave->hSc->blockSignals(false);
    }
    else if(event->pos().x() > _ConSpec->pos().x() && event->pos().x()<(_ConSpec->width()+_ConSpec->pos().x()))
    {
      double p = event->pos().x()-_ConSpec->pos().x();
      p = (float)p/_ConSpec->hSc->width() * 100 -50;
      _DisSpec->hSc->blockSignals(true);
      _ConSpec->hSc->blockSignals(true);
      bindSpecScroll(p);
      _DisSpec->hSc->blockSignals(false);
      _ConSpec->hSc->blockSignals(false);
      _DisSpec->getWidget()->setHshift(p);
      _ConSpec->getWidget()->setHshift(p);
    }
  }
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
        buf1[i] =_orgData[i];
    utilities->updateUtilites(buf1,num);
    _orgFileName=QDir::tempPath()+"/tmp.wav";
    ria->writeToWave(buf1,_orgFileName.toLatin1().data(),sr,num);
    _ConSpec->getWidget()->updateGL();
     nonIntSr(sampleRateSelect->currentText());
    _DisSpec->getWidget()->updateGL();
    _OrgWave->getGlwidget()->updateGL();
    _SampledWave->getGlwidget()->updateGL();
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
  for(int i =0; i < num; i++)
      buf1[i] =(_orgData[i] + (float)sin(freq*i)/5.0)/1.2;
  utilities->updateUtilites(buf1,num);
  _orgFileName=QDir::tempPath()+"/tmp.wav";
  _sampleFileName = _orgFileName;
  ria->writeToWave(buf1,_orgFileName.toLatin1().data(),sr,num);
   nonIntSr(sampleRateSelect->currentText());
  _ConSpec->getWidget()->updateGL();
  _DisSpec->getWidget()->updateGL();
  _OrgWave->getGlwidget()->updateGL();
  _SampledWave->getGlwidget()->updateGL();
}
