#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include<QSpinBox>
#include"audiohandle.h"
#include<QComboBox>
#include"plotfilter.h"
#include<QCheckBox>
#include "utilities.h"
#include"plot.h"
class QAction;
class QMenu;
using namespace Phonon;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
     plot* _SampledWave;
     float *buf1;
     plotFilter* _FilterWidget;
     plot *_DisSpec;
     plot *_OrgWave;
     plot *_ConSpec;
     QSpinBox *bitBox;
     int currentNum;
     int L;
     int down;
     float* sampleData;
     float* qData;
     void loadOrgwave();
     void loadSample();
     void loadCSpec();
     void loadDSpec();
     void updateFilter();
     float getMax(float *a,int num);

 public   slots:
    void play();
    void playSample();
    void playfiltered();
    void open();
    void about();
    void showFilter(bool i);
    void plot2Freq();
    void onlyCon();
    void onlyDis();
    void nonIntSr(QString s);
    void applyFilters();
    void resetW1();
    void bindWaveScroll(int s);
    void resetSp();
    void bindSpecScroll(int s);
    void vresetW1();
    void vbindWaveScroll(int s);
    void vresetSp();
    void vbindSpecScroll(int s);
    void setBits(int bits);
    void frozenScaling(bool i);
    void displayAliasing(bool i);
    void wheelEvent(QWheelEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enableNoiseSelect(bool i);
    void noiseAddfunc(int index);

private:
    int bits;
    void createActions();
    void createMenus();
    void filter();
    void loadFile(const QString &fileName);
    void createWidget(QWidget *main);
    int f;
    int sr;
    int c;
    int num_items;
    float time;
    int num;
    int gcd(int v1,int v2);
    bool discrete;
    bool appFilter;
    float ripple;
    bool Butter;
    float *filterData;
    int order;
    void aafilter();
    void getFilterdata();
    void initializeVar();
    QString fileName;
    QString fileName1;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *openAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    AudioHandle *ria;
    QWidget *main;
    QString filename;
    QComboBox *sampleRateSelect;
    bool onlyfiltered;
    QCheckBox* show2Spec;
    QCheckBox* conFirst;
    QCheckBox* disFirst;
    QLabel* SNR;
    Utilities* utilities;
    void resetEverything();
    float* tmpData;
    bool ifShowAlias;
    bool alreadyFiltered;
    int _trueBits;
    QCheckBox* _selFilter;
    QCheckBox* _showAliasing;
    QCheckBox* _showSampleRange;
    QCheckBox* _showOnePeriod;
    bool loadingFailed;
    QCheckBox* _addNoise;
    QComboBox* _selectNoise;
    float* _orgData;
};

#endif // MAINWINDOW_H
