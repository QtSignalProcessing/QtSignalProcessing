#ifndef GLSW_H
#define GLSW_H
#include"glbase.h"
#include <QGLWidget>
#include<QString>
#include "axislabel.h"
class GLSW : public GLBase
{
    Q_OBJECT
public:
    GLSW(float *buf,int num,float time,bool sample,QWidget *parent=0);
    ~GLSW();
    void plotAxis();
    void ygenerateAxis();
    void setSampleFactors(float SNR,int bits);
    GLSW* getGlwidget();
protected:
    void paintGL();
 signals:
    void newSNR(double SNR);
public slots:   
    void setSNR(double SNR);
    void setStickOn(bool i);
private:
    float SNR;
    float r;
    void computeCurrent();
    inline int getIndex(float value);
    bool _stickOn;
};

#endif // GLSW_H
