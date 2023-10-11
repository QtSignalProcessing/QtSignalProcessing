#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <QMainWindow>

class QAction;
class QMenu;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;


class PlotWidget;
class plotFilter;
class AudioHandle;
class Utilities;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
      MainWindow(QWidget *parent = 0);
      ~MainWindow();
      void updateFilter();
      float getMax(float *a,int num);
      QVector<float> pointer2Qvec(float* data,int size);

signals:
      void OrderTooHigh(bool);
      void RippleNotValid(bool);

  public slots:
      void play();
      void playSample();
      void playfiltered();
      void open();
      void about();
      void aboutDefaultSignal();
      void showFilter(bool i);
      void plot2Freq();
      void onlyCon();
      void onlyDis();
      void nonIntSr(int s);
      void applyFilters();
      void setBits(int bits);
      void displayAliasing(bool i);
      void enableNoiseSelect(bool i);
      void noiseAddfunc(int index);
      void butterOrCheby(int index);
      void setOrder(QString order);
      void setNyqFreq();
      void setRipple(QString ripple);
      void setFactor(QString s);

  private:
      struct FilterPara
      {
          bool butter;
          int order;
          float* filterData;
          float ripple;
      };

     struct SamplePara
     {
         int currentNum;
         int upSampleFactor;
         int downSampleFactor;
         float* sampleData;
     };

      PlotWidget* _SampledWave;
      float *_data;
      plotFilter* _FilterWidget;
      PlotWidget *_DisSpec;
      PlotWidget *_OrgWave;
      PlotWidget *_ConSpec;
      QSpinBox *bitBox;
      float* _quantizedData;
      int bits;
      void createActions();
      void createMenus();
      void filter();
      void loadFile(const QString &fileName);
      void createWidget(QWidget *main);
      void setFilterDataToWidgets();
      int sr;
      float time;
      int num;
      void aafilter();
   //   void getFilterdata();
      void initializeVar();
      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *openAct;
      QAction *exitAct;
      QAction *aboutAct;
      QAction *aboutDefaultSignalAct;
      QAction *aboutQtAct;
      AudioHandle *ria;
      QWidget *main;
      QComboBox *sampleRateSelect;
      QCheckBox* show2Spec;
      QCheckBox* conFirst;
      QCheckBox* disFirst;
      QLabel* SNR;
      Utilities* utilities;
      void resetEverything();
      float* tmpData;
      bool ifShowAlias;
      int _trueBits;
      QCheckBox* _selFilter;
      QCheckBox* _showAliasing;
      QCheckBox* _showSampleRange;
      QCheckBox* _showOnePeriod;
      bool loadingFailed;
      QCheckBox* _addNoise;
      QComboBox* _selectNoise;
      float* _orgData;
      QString _orgFileName;
      QString _sampleFileName;
      QString _filteredFileName;
      QVector<int> _samplingRates;
      FilterPara _filterData;
      SamplePara _sampleData;
};

#endif // MAINWINDOW_H
