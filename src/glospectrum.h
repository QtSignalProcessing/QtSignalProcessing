#ifndef GLOSPECTRUM_H
#define GLOSPECTRUM_H

#include"glbase.h"

class GLOSpectrum : public GLBase
{
    Q_OBJECT
public:
    GLOSpectrum(float *buf,int num,bool discrete,int sr,QWidget *parent = 0);
    ~GLOSpectrum();
    float factor;//current sampling rate propotional to the Nyquist rate;
    bool showRange;//flag of showing the sampling range
    float* filterData;//filtered data
    double NyqFreq; //cutoff freq propotional to the Nyquist rate to avoid aliasing
    int order; //order of the filter
    bool showFilter;
    int currentIndex;
    float ripple;
    bool conFirst;
    bool disFirst;
    bool show;
    int yTimeOfScale;
    bool discrete;
    float sr;
    float xMin;
    float xMax;
    float yaxisFactor;
    bool show2Spec;
    bool butter;
    void generateAxis();
    void plotAxis(float axisFactor);
    void plotAxis();
    void plotAxis(bool discrete);
    void ygenerateAxis();
    void yPlotAxis(float yaxisFactor);
    float* yCor;
    int getIndex(float value);
    int normalizeMax(float max);
    int nmax;
    void XplotAxis();
    float inline getValue(float xCor);
    float inline getCor(float xValue);
    void setBanned(bool i);
    bool get1Period();
    GLOSpectrum* getWidget();

protected:
    void paintGL();

private:
    double* getAmplitude(int num, double *in);
    void computeCurrent();
    void plotXAxis();
    bool showOnePeriod;
    int numofP;
    float yScales;
    double* totalValue;
    double* totalValue1;
    double* amplitude;
    double* amplitude1;
    double* amplitude2;
    bool showAliasing;
    bool getAlias;
    int timeOfScale;
    bool banned;
    inline int getIndex1(float value);
    int _currentYmax;
    QVector<float> ycord;
    QVector<float> yl;
signals:
   void OrderTooHigh(bool);
   void RippleNotValid(bool);
   void TooManyPeriods(bool);
public slots:
    void resetV();
    void vIncrease();
    void vDecrease();
    void setVshift(int shift);
    void hDecrease();
    void hIncrease();
    void resetH();
    void nonIntSr(QString s);
    void showSampleRange(int state);
    void setFactor(QString s);
    void setOrder(QString order);
    void Filter();
    void setNyqFreq(QString s);
    void setToN();
    void show2F(bool state);
    void conDis();
    void disCon();
    void butterOrCheby(int index);
    void setRipple(QString ripple);
    void setCutofFreq(QString s);
    void resetSpec(float* data,int number,int sr);
    void show1Period(bool i);
    void setShowAliasing(bool i);
};

#endif // GLOSPECTRUM_H
