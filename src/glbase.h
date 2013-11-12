#ifndef GLBASE_H
#define GLBASE_H
#include <QGLWidget>
#include<QString>
#include "axislabel.h"
#include<math.h>
class GLBase : public QGLWidget
{
    Q_OBJECT
public:
    GLBase(float *buf,int num,float time,bool sample,int sr,QWidget *parent=0);
    GLBase(float *buf,int num,QWidget *parent = 0);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void resetCommon();
    void resetDataAndOther(float* data,int num,float time,float max);
    void generateAxis();
    void plotAxis();
    void ygenerateAxis();
    void setFacors(int orgShift);
    void setSampleFactors(float SNR,int bits);
    void setL(int L,int currentNumItems,int downSample,bool replace);
    int getMaximun();
    float getMax1(float *a,int num);
    void updateLabel(double max,double min);
    int sRate;
    float getMax1(double *a,int num);
    int sampleRate;
    AxisLabel* xlabel;
    AxisLabel* ylabel;
    float currentYMax;
    float currentYmin;
    float yMaxcord;
    float yMincord;
    int gcd(int v1,int v2);
    void computeCurrent();
    void computeYCurrent();
    int width;
    int height;
    double downSample;
    double vIn;
    double vDe;
    float hIn;
    int hshift;
    int vshift;
    int bits;
    int L;
    float currentMin;
    float currentMax;
    bool sampleOrNot;
    int sr;
    float time;
    int number;
    int currentNumItems;
    float* data;
    float *plotData;
    float max;
    int orgShift;
    float* xLabels;
    float* xCor;
    float* yLabels;
    float* yCor;
    float xScaleW1;//scaler factor of the x-axis of the waveform
    float xShiftW1;// Shift the x-axis of the waveform(left and right)
    float yScaleW1;//scaler factor of the y-axis of the waveform
    float yShiftW1;// Shift the y-axis of the waveform (up and down)
    float xFactor2;
    bool replace;
    float getDist(float x1,float y1,float x2,float y2);

protected:
    void initializeGL();
    virtual void paintGL()=0;
    void resizeGL(int width, int height);

public slots:
    void vIncrease();
    void vDecrease();
    void hIncrease();
    void hDecrease();
    void setHshift(int shift);
    void setVshift(int shift);
    void resetV();
    void resetH();
    void setSampleRate(int sr);
    void setBits(int bits);
    void nonIntSr(QString s);
};

#endif // GLBASE_H
