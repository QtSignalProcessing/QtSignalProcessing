#ifndef GLSPECTRUM_H
#define GLSPECTRUM_H

#include "glwidgetnew.h"

class GLSpectrum : public GLWidgetnew
{
    Q_OBJECT
public:
    GLSpectrum(const QVector<float>&data,bool isSample,double ratio,int sampleRate,QWidget* parent=0);
    void setRatio(double ratio);
    void updateY();
    void setDataForAliasing(QVector<float>& data);
    void setDataFor2Freq(QVector<float>& data);
    void updateSampleRate(int sampleRate);

protected:
    void paintGL();
    void yLabel();
    void xLabel();

public slots:
    void show1Period(bool i);
    void showAliasing(bool i);
    void showSampleRange(int i);
    void show2F(bool state);
    void conDis(bool state);
    void disCon(bool state);
    void vIncrease();
    void vDecrease();

private:
    double _ratio;
    bool _onePeriod;
    bool _showAliasing;
    QVector<float> _dataForAliaing;
    bool _showRange;
    double _rangeRatio;
    QVector<float> _dataFor2Freq;
    bool _plot2Freq;
    bool _conFirst;
    int _sampleRate;
    float _ystep;

    void setScalingX(double scale);
    void setScalingY(double scale);
    void plotdata(const QVector<float>data);
    void plotRange();
    void plot2Freq();
};

#endif // GLSPECTRUM_H
