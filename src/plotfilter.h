#ifndef PLOTFILTER_H
#define PLOTFILTER_H

#include <QWidget>

class GLWidgetnew;
class GLSpectrum;

class QLabel;
class QLineEdit;
class QComboBox;
class QToolButton;

class plotFilter : public QWidget
{
    Q_OBJECT
  public:
    plotFilter(float *ria,int num,bool discrete,int sr,QVector<float>& specdata,QWidget *parent = 0);
    GLWidgetnew* getWaveWidget();
    GLSpectrum* getSpecWidget();
    QComboBox* getFilterSelect();
    QLineEdit* getNumOfOrder();
    QLineEdit* getRippleEdit();
    QLabel* getOrderLabel();
    QLabel* getRippleLabel();
    QComboBox* getCutFreq();
    QToolButton* getSetTonyqB();
    QToolButton* getApplyButton();
    QStringList texts;
    QToolButton* play;
  signals:
    void closed(bool);

  public slots:
    void filterChanged(int index);
    void setComboText(QString string);
    void setCutFreqToNy();
    void setActualFreq();

  private:
    QString NqLabel;
    QLabel* OrderIsToHigh;
    QLabel* RippleNotValid;
    QLabel* name;
    QFont font;
    QLabel* Fdes;
    QLabel* order;
    QWidget* ooder;
    QLabel* cutOfF;
    void closeEvent(QCloseEvent *event);
    QLabel *_displayActualFreq;
    GLWidgetnew* _wavewidget;
    GLSpectrum* _specwidget;
    QLabel* rip;
    QComboBox *cutFreqSel;
    float NyqFreq;

    int sr;
    QToolButton* apply;
    QToolButton* reset;

    QComboBox *filterSelect;
    QLineEdit* numOfOrder;
    QLineEdit* ripple;
};

#endif // PLOTFILTER_H
