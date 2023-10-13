#ifndef GLSPECTRUM_H
#define GLSPECTRUM_H

#include "glwidgetnew.h"

class GLSpectrum : public GLWidgetnew
{
    Q_OBJECT
public:
    GLSpectrum(const QList<float>& data,bool is_sample,double ratio,int sample_rate,QWidget* parent=0);
    void setRatio(double ratio);
    void updateY();
    void setDataForAliasing(QList<float>& data);
    void setDataFor2Freq(const QList<float>& data);
    void updateSampleRate(int sample_rate);

protected:
    void paintGL();
    void yLabel();
    void xLabel();
    void xrenderText(double x, double y, const QString text);

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
    QList<float> _dataForAliaing;
    bool _showRange;
    double _rangeRatio;
    QList<float> _dataFor2Freq;
    bool _plot2Freq;
    bool _conFirst;
    int _sampleRate;
    float _ystep;

    void setScalingX(double scale);
    void setScalingY(double scale);
    void plotdata(const QList<float>data);
    void plotRange();
    void plot2Freq();
};

#endif // GLSPECTRUM_H
