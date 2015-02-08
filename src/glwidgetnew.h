#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

class GLWidgetnew : public QGLWidget
{
    Q_OBJECT
public:
    GLWidgetnew(const QVector<float>&data,float time,bool isSample,QWidget* parent=0);
    void setData(const QVector<float>&data);
    void updateMax();
    float getMax();
    void setOffset(int offset);
    bool getIsSample();
    void setTime(float time);

signals:
   void newSNR(double SNR);
   void xAxisScaled(double scalef);
   void xAxisShifted(double shiftx);

 public slots:
   void setSNR(double SNR);
   void setStickOn(bool i);
   void resetH();
   void setLinkedScale(double scale);
   void setLinkedShift(double shift);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    //float getMax();
    float getMin();
    void drawAxis();
    void yLabel(int offset = 35);
    void xLabel();

    void wheelEvent(QWheelEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    float _shiftX;
    float _shiftY;
    int _shiftXstart;
    int _shiftYstart;
    bool _mousePressed;
  //  bool _locked;
    bool _linked;
    int _xMouse;
    int _yMouse;
    double _yscalingF;
    double _xscalingF;
    double _mousePressedX;
    double _mousePressedY;
    double _scalingFactorX;
    double _scalingFactorY;
    double _xscaleF;
    double _yscaleF;
    double _xmin;
    int _xboarder;
    int _yboarder;
    double _Xmax;
    double _Xmin;
    double _Ymax;
    double _Ymin;
    int _xGrid;
    int _yGrid;
    QVector<float> _data;
    float _step;
    int _height;
    int _width;
    bool _isSample;
    bool _stickon;
    float _time;
    int _offset;

private:



     virtual  void setScalingX(double scale);
     float getDist(float x1,float y1,float x2,float y2);

     void DrawCircle(float cx, float cy, float r, int num_segments) ;
};




#endif // GLWIDGET_H
