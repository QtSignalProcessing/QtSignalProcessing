//#include <qwt_plot.h>
#include <qdatetime.h>
#ifndef QWTPLOT_H
#define QWTPLOT_H




class Curve;

class qwtPlot: public QwtPlot
{
public:
    qwtPlot( QWidget * = NULL);

protected:
    virtual void timerEvent( QTimerEvent * );

private:
    void updateCurves();

    enum { CurveCount = 4 };
    Curve *d_curves[CurveCount];

    QTime d_time;
};

#endif // QWTPLOT_H
