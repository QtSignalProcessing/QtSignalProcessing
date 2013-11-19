#include "axislabel.h"

#include <cstdlib>
#include <cmath>

static const double _eps = 1.0e-6;

AxisLabel::AxisLabel(double min,double max):size(0),steps(0),min(min),max(max),maxNumSteps(8),currentMin(min),currentMax(max),
    base(10),labels(NULL),cord(NULL),yindex(NULL),ycord(NULL)
{
    range = max - min;
    if( range < 0 )
        range = -range;
}

AxisLabel::~AxisLabel()
{
    if(labels != NULL)
        free(labels);
    if(cord != NULL)
        free(cord);
    if(yindex != NULL)
        free(yindex);
    if(ycord != NULL)
        free(ycord);
}

void AxisLabel::autoScale()
{
    divideInterval();
    size = ceil( range / steps );
}

void AxisLabel::divideInterval()
{
    double intervalSize = currentMax - currentMin;
    if(maxNumSteps <= 0 )
    {
        steps = 0;
        return ;
    }
    const double v=(intervalSize - ( _eps * intervalSize ) ) / maxNumSteps;

    if( v == 0.0 )
    {

        steps = 0;
        return;

    }
    const double lx = log(fabs(v))/log((double)base);
    const double p = floor(lx);
    const double fraction = pow( base, lx-p );
    float n = base;
    while( ( n > 1 ) && ( fraction <=n / 2 ) )
        n /= 2;

    steps = n * pow( base , p );
    if( v < 0 )
        steps = -steps;

}

void AxisLabel::setCurrent(double currentMin, double currentMax)
{
    this->currentMax = currentMax;
    this->currentMin = currentMin;
}

float* AxisLabel::getLabels()
{
    if( labels != NULL )
        free( labels );
    labels = ( float* )malloc( sizeof( float ) * size );
    for( int i = 0; i< size; i++ )
    {
        labels[i] = min + steps * (float)( i + 1 );
    }
    return labels;
}

float* AxisLabel::getLabels(bool spec)
{
    if(spec)
    {
    if( labels != NULL )
        free( labels );

    labels = ( float* )malloc( sizeof( float ) * size );
    for( int i = 0; i< size ; i++ )
    {
        labels[i] = max - steps * (float)( i + 1 );
    }
    }
    else
    {
        if( labels != NULL )
            free( labels );

        labels = ( float* )malloc( sizeof( float ) * size );
        for( int i = 0; i< size ; i++ )
        {
            labels[i] = min + steps * (float)( i  );
        }
        }

    return labels;
}

float* AxisLabel::getCord(float time, int number,float xScale,float xShift, double hshift, int width, float hIn)
{
    if(cord != NULL)
        free( cord );
    cord = ( float* )malloc( sizeof( float ) * size );
    for( int i = 0; i < size; i++ )
    {
        cord[i]=(labels[i] / time * number)/(xScale*hIn)-hshift+((width-xShift)/2-(number/2)/(xScale*hIn))+xShift;
    }
    return cord;

}

void AxisLabel::setMaxNumSteps(int maxNumSteps)
{
    this->maxNumSteps = maxNumSteps;
}

float* AxisLabel::getYcord(float yscale,float vIn,float yshift,float vshift)
{
    if(ycord != NULL )
        free(ycord);
     ycord = ( float* )malloc( sizeof( float ) * (size ) );
    for(int i = 0 ; i< size; i++)
        ycord[i] = labels[i]*(yscale*vIn)+yshift+vshift;
    return ycord;
}

void AxisLabel::updateMaxMin(double max, double min)
{
    this->max = max;
    this->min = min;
    setCurrent(max,min);
    range = max - min;
    if( range < 0 )
        range = -range;
}
