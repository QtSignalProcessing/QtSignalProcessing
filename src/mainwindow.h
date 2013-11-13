#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;

class plot;
class plotFilter;
class AudioHandle;
class Utilities;

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
      void enableNoiseSelect(bool i);
      void noiseAddfunc(int index);
      
  protected:
    void mouseMoveEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent * event);

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
      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *openAct;
      QAction *exitAct;
      QAction *aboutAct;
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
      QString _orgFileName;
      QString _sampleFileName;
      QString _filteredFileName;
};

#endif // MAINWINDOW_H
