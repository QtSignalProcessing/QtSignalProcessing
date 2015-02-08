#include "glspectrum.h"

#include <math.h>

#include <QDebug>

GLSpectrum::GLSpectrum(const QVector<float>&data,bool isSmaple,double ratio,int sampleRate,QWidget* parent):GLWidgetnew(data,0,isSmaple,parent),
    _ratio(ratio),_showAliasing(false),_dataForAliaing(),_showRange(false),_rangeRatio(1.0),_dataFor2Freq(),_plot2Freq(false),_conFirst(false),_sampleRate(sampleRate),_ystep(100)
{
    _Ymax += 0.01 * _Ymax;
}

void GLSpectrum::paintGL()
{
    _Ymin = 0;
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 0.0);
    if(_isSample)
        glColor3f(0,0,1);
    else
        glColor3f(0,0,0);
    _Xmax = _data.size() / _ratio;
    _xscaleF = (double)(this->width()-_xboarder)/(double)(_Xmax);
    _yscaleF = (double)(this->height()-_yboarder)/(double)(_Ymax-_Ymin);
    //set new max/min values for free space at the widget borders
    _xmin = _Xmin - _xboarder/_xscaleF;
    drawAxis();
    glTranslatef(float(_shiftX),float(_shiftY),0);
    glScalef(_xscalingF,_yscalingF,0);

    yLabel();
    xLabel();
    if(_showRange)
        plotRange();
    if(!_isSample)
    {
        glBegin(GL_LINE_STRIP);
        for( int i = _data.size()-1; i > 0; i-- )
        {
            float xCor = (i-_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2;
            if( xCor  * _xscalingF + _shiftX  >= _xboarder   && xCor * _xscalingF + _shiftX <= _width )
                glVertex2f( xCor, (_Ymax-_data[i]*10)*_yscaleF );
        }
        glEnd();

        if(_xscalingF< 1.0 )
        {
        glBegin(GL_LINES);
        glVertex2f((-_xmin*_xscaleF-_shiftX)/_xscalingF  ,(_Ymax)*_yscaleF);
        glVertex2f(_xboarder,(_Ymax)*_yscaleF);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f((_data.size()-_xmin*_xscaleF-_shiftX)/_xscalingF  ,(_Ymax)*_yscaleF);
        glVertex2f(_width,(_Ymax)*_yscaleF);
        glEnd();
        }
    }
    else
    {
        if(_plot2Freq && !_conFirst)
        {
            plot2Freq();
        }
        if(_showAliasing)
        {
            glColor3f(1,0,0);
        }
        plotdata(_data);
        if(_showAliasing)
        {
            glColor3f(0,0,1);
            plotdata(_dataForAliaing);
        }
        if(_plot2Freq && _conFirst)
        {
            plot2Freq();
        }
    }
    glPopMatrix();
}

void GLSpectrum::setScalingX(double scale)
{
  double oldScale = _xscalingF;
  if(scale == oldScale)
  {
    return;
  }
  _xscalingF = scale;
  if(scale < 1.0)
  {
      int size;
      if(_ratio<1.0)
          size = _data.size()/_ratio;
      else
          size = _data.size();
     _shiftX = (double)(this->width()-_xboarder)/2 - (size/2 - _shiftX) * _xscaleF * _xscalingF;
     if(_linked)
     {
         emit xAxisShifted(_shiftX);
         emit xAxisScaled(_xscalingF);
     }
      updateGL();
      return;
  }

   double xcenter = (_xMouse-_shiftX+_xmin*_xscaleF)/_xscaleF/oldScale;
  _shiftX -= qRound(xcenter*(_xscalingF-oldScale)*_xscaleF);
  if(_shiftX < _Xmax*_xscaleF*(1-_xscalingF))
  {
    _shiftX = _Xmax*_xscaleF*(1-_xscalingF);
  }
  if(_shiftX > 0 || _xscalingF==1.0)
  {
    _shiftX = 0;
  }
  if(_linked)
  {
    emit xAxisShifted(_shiftX);
    emit xAxisScaled(_xscalingF);
  }
  updateGL();
}

void GLSpectrum::show1Period(bool i)
{
    _onePeriod = i;
    updateGL();
}

void GLSpectrum::setRatio(double ratio)
{
    if(_isSample)
    {
        _ratio = ratio;

    }
    else
        _rangeRatio = ratio;
    updateGL();
}

void GLSpectrum::updateY()
{
    _Xmax = _data.size();
    _Ymax = getMax()*10;
    _Ymin = getMin()*10;
}

void GLSpectrum::showAliasing(bool i)
{
    _showAliasing = i;
    updateGL();
}

void GLSpectrum::setDataForAliasing(QVector<float> &data)
{
    if(_dataForAliaing.size()!=0)
        _dataForAliaing.clear();
    if( data.size() > _data.size() )
    {
        int num = data.size() - _data.size();
        for( int i = num/2 + 1; i < data.size() - num/2-1; i++ )
            _dataForAliaing.push_back(data[i]*_ratio);
        _dataForAliaing.push_front(0);
        _dataForAliaing.push_back(0);
    }
    else
         _dataForAliaing = data;
}

void GLSpectrum::setDataFor2Freq(QVector<float> &data)
{
    if(_dataFor2Freq.size()!=0)
        _dataFor2Freq.clear();
    _dataFor2Freq = data;
}

void GLSpectrum::plotdata(const QVector<float> data)
{
    glBegin(GL_LINE_STRIP);
    for( int i = data.size()-1; i >= 0; i-- )
    {
        float xCor = (i-_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2;
       // if(i==0)
           // qDebug()<<xCor<<" "<<(1-_ratio) * (double)(this->width()-_xboarder)/2<<" "<<(i-_xmin)*_xscaleF<<" "<<_xmin;
        if( xCor  * _xscalingF + _shiftX  >= _xboarder   && xCor * _xscalingF + _shiftX <= _width  )
        {
             glVertex2f( xCor, (_Ymax-data[i]*10)*_yscaleF );
        }
    }
    glEnd();
    if(!_onePeriod)
    {
        int i = 0, j = 0;
        float secEnd = ((data.size()-1-_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2) * _xscalingF + _shiftX;
        float con = 0;
        if( _xscalingF >= 1.0 )
        {
            con = (_width + _xboarder) * _xscalingF  -  secEnd - ((i-_xmin)*_xscaleF * _xscalingF );
        }
        else
            con = (_width + _xboarder)  -  secEnd - ((i-_xmin)*_xscaleF * _xscalingF );

        glBegin(GL_LINE_STRIP);
        while( con > 0 )
        {
          //  if( (i-_xmin)*_xscaleF  * _xscalingF + _shiftX >= _xboarder  && (i-_xmin)*_xscaleF  * _xscalingF + _shiftX  <= _width )
            {
                 glVertex2f( (i + data.size()-1 -_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2 , (_Ymax-data[j]*10)*_yscaleF );
            }
            i++;
            j++;
            if( j >=data.size()-1 )
                j = 0;
            if( _xscalingF >= 1.0 )
            {
                con = (_width + _xboarder) * _xscalingF  -  secEnd - ((i-_xmin)*_xscaleF * _xscalingF );
            }
            else
                con = (_width + _xboarder)  -  secEnd - ((i-_xmin)*_xscaleF * _xscalingF );
        }
        glEnd();
        i = 0;
        j = data.size()-1;
        glBegin(GL_LINE_STRIP);
        float firEnd = ((-_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2) * _xscalingF ;

        if( _xscalingF < 1 )
            con = firEnd - ((i-_xmin)*_xscaleF * _xscalingF) + _shiftX ;
        else
            con = firEnd - ((i-_xmin)*_xscaleF * _xscalingF) - _shiftX;

        while( con  > 0 )
        {
            if( ((-i-_xmin)*_xscaleF+ (1-_ratio) * (double)(this->width()-_xboarder)/2)  * _xscalingF + _shiftX >= _xboarder  && ((-i-_xmin)*_xscaleF+ (1-_ratio) * (double)(this->width()-_xboarder)/2)  * _xscalingF + _shiftX  <= _width )
            {
            glVertex2f(  ( -i -_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2 , (_Ymax-data[j]*10)*_yscaleF );
            }
            i++;
            j--;
            if( j <= 0 )
                j = data.size()-1;

            if( _xscalingF < 1 )
                con = firEnd - ((i-_xmin)*_xscaleF * _xscalingF) + _shiftX ;
            else
                con = firEnd - ((i-_xmin)*_xscaleF * _xscalingF) - _shiftX ;
        }
        glEnd();
    }
}

void GLSpectrum::showSampleRange(int i)
{
    if(i==0)
    {
        _showRange=false;
    }else{
        _showRange=true;
    }
    updateGL();
}

void GLSpectrum::plotRange()
{
    float ratio;
    float size;
    if(_isSample)
    {
        ratio = _ratio;
        size = _data.size();
    }
    else
    {
        ratio = _rangeRatio;
        size = _data.size() * _rangeRatio;
    }
    float xCor1 = ((-_xmin)*_xscaleF + (1-ratio) * (double)(this->width()-_xboarder)/2) ;
    float xCor2 = ((size-_xmin)*_xscaleF + (1-ratio) * (double)(this->width()-_xboarder)/2) ;

    if(_isSample)
    {
        glBegin(GL_POLYGON);
        glColor3f(0,1,0);
        glVertex2f(xCor1, 0);
        glVertex2f(xCor2, 0);
        glVertex2f(xCor2, _height-_yboarder);
        glVertex2f(xCor1, _height-_yboarder);
        glEnd();
    }
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex2f(xCor1, 0);
    glVertex2f(xCor1, _height-_yboarder);
    glVertex2f(xCor2, 0);
    glVertex2f(xCor2, _height-_yboarder);
    glEnd();
    if(_isSample)
        glColor3f(0,0,1);
    else
        glColor3f(0,0,0);
}

void GLSpectrum::plot2Freq()
{
    double xmax = _dataFor2Freq.size();
    double xscaleF = (double)(this->width()-_xboarder)/(double)(xmax);
    float xmin =  - _xboarder/xscaleF;
    glColor3f(1,0,0);
    glBegin(GL_LINE_STRIP);
    for( int i = _dataFor2Freq.size()-1; i >= 0; i-- )
    {
        float xCor = (i-xmin)*xscaleF;
       // if(i==0)
           // qDebug()<<xCor<<" "<<(1-_ratio) * (double)(this->width()-_xboarder)/2<<" "<<(i-_xmin)*_xscaleF<<" "<<_xmin;
      //  if( xCor  * _xscalingF + _shiftX  >= _xboarder   && xCor * _xscalingF + _shiftX <= _width  )
        {
             glVertex2f( xCor, (_Ymax-_dataFor2Freq[i]*10*_ratio)*_yscaleF );
        }
    }
    glEnd();
    glColor3f(0,0,1);
}

void GLSpectrum::show2F(bool state)
{
    _plot2Freq = state;
    updateGL();
}

void GLSpectrum::conDis(bool state)
{
    _conFirst = state;
    updateGL();
}

void GLSpectrum::disCon(bool state)
{
    _conFirst = !state;
    updateGL();
}

void GLSpectrum::yLabel()
{
    if(_isSample)
    {
        GLWidgetnew::yLabel(45);
    }
    else
    {
        float max = _Ymax*_ratio/_sampleRate;
     //   qDebug()<<_ratio<<" "<<_Ymax;
      /*  uint n = height()/20*_yscalingF;
        if (n < 1) { n = 1; }
        double S[]={1,2,5};
        uint i = 0;
        uint grid = 1;
        while(!((int)(n*grid)>(max-_Ymin)))
        {
          ++i;
          grid = pow(10,i/3)*S[i%3];
        }
        _ystep = grid;
       // int num = max/_ystep;
        _yGrid = _Ymax/num;*/


        const double v=(max - ( 1.0e-6 * max ) ) / 8.0;
        if( v == 0.0 )
        {
            _ystep = 0;
        }
        const double lx = log(fabs(v))/log((double)10);
        const double p = floor(lx);
        const double fraction = pow( 10, lx-p );
        float m = 10;
        while( ( m > 1 ) && ( fraction <=m / 2 ) )
            m /= 2;
        _ystep = m * pow( 10 , p );
        if( v < 0 )
            _ystep = -_ystep;
        int num = max / _ystep;
        _yGrid = _Ymax/num;
      //  double ratio = _ratio*_sampleRate;
       //   qDebug()<<_yGrid<<" "<<max<<" "<<_Ymax<<" "<<_sampleRate<<" "<<_ystep<<" "<<num<<" "<<_Ymin;
        glScalef(1.0/_xscalingF,1.0/_yscalingF,0);
        //draw y-grid
        QFont f;
        f.setPixelSize(10);
        double tmp = 1000000;
      //  double  yscaleF = (double)(this->height()-_yboarder)*ratio/(double)(max-_Ymin);
        int k =0;
        for (int i = _Ymin; i <= _Ymax; i = i + _yGrid)
        {
            double yPos = (_Ymax-i)*_yscaleF ;
          double xPos = -_xmin*_xscaleF-_shiftX;
       //   if(yPos < (max-_Ymin)*_yscaleF-_shiftY+1 && tmp - yPos > 20 )
          {
              glBegin(GL_LINES);
              glVertex2f(xPos-2, yPos);
              glVertex2f(xPos+2, yPos);
              glEnd();
             renderText (xPos - 25,yPos+5 , 0, QString::number(k*_ystep),f);
             tmp = yPos;
             k++;
          }
        }
        glScalef(_xscalingF,_yscalingF,0);
    }
}

void GLSpectrum::xLabel()
{
    QFont f;
    f.setPixelSize(10);
    float max;
    if(!_isSample)
    {
        max = (float)_sampleRate/1000.0;
    }
    else
        max = 2.0;
    if(_xscalingF >=1&&_ratio>=1)
    {
        int index1 = (_xboarder - _shiftX) / _xscalingF / _xscaleF + _xmin;
        double currentMin = (float)index1 / _data.size() * max;
        int index2 = (_width - _shiftX) / _xscalingF / _xscaleF + _xmin;
        double currentMax = (float)index2 / _data.size() * max;
        double intervalSize = currentMax - currentMin;
        const double v=(intervalSize - ( 1.0e-6 * intervalSize ) ) / 8.0;
        if( v == 0.0 )
        {
            _step = 0;
        }
        const double lx = log(fabs(v))/log((double)10);
        const double p = floor(lx);
        const double fraction = pow( 10, lx-p );
        float m = 10;
        while( ( m > 1 ) && ( fraction <=m / 2 ) )
            m /= 2;
        _step = m * pow( 10 , p );
        if( v < 0 )
            _step = -_step;
        int num = max / _step;
        if(num%2 != 0)
            num+=1;
        _xGrid = floor(_Xmax / num);
        if(_xGrid < 1)
            _xGrid = 1;
        int digit = round(fabs(log10 ( _step)-1));
        int k = 0;
        int xoffset = 15;
        for (int i = 0; i < _Xmax/2; i += _xGrid)
        {
            double xPos = (i+_data.size()/2-_xmin)*_xscaleF;
            double yPos = (_Ymax-_Ymin)*_yscaleF-_shiftY;
          if(xPos * _xscalingF + _shiftX >= _xboarder && xPos * _xscalingF + _shiftX <= _width)
          {
            glBegin(GL_LINES);
            glVertex2f(xPos, yPos-2);
            glVertex2f(xPos, yPos+2);
            glEnd();
            renderText (xPos - 10/_xscalingF ,yPos+15 , 0, QString::number(k*_step,'f',digit),f);
            k++;
          }
        }
        k = -1;
        for (int i = _Xmax/2 - _xGrid; i >=0; i -= _xGrid)
        {
            double xPos = (i-_xmin)*_xscaleF;
            double yPos = (_Ymax-_Ymin)*_yscaleF-_shiftY;
          if(xPos * _xscalingF + _shiftX >= _xboarder && xPos * _xscalingF + _shiftX <= _width)
          {
            glBegin(GL_LINES);
            glVertex2f(xPos, yPos-2);
            glVertex2f(xPos, yPos+2);
            glEnd();
            renderText (xPos - xoffset/_xscalingF ,yPos+15 , 0, QString::number(k*_step,'f',digit),f);
            k--;
          }
        }
    }
    else if(!_isSample && _xscalingF < 1.0)
    {
        float secEnd = (_data.size()-1 -_xmin)*_xscaleF;
        float firEnd = ((-_xmin)*_xscaleF) ;
        double yPos = (_Ymax-_Ymin)*_yscaleF-_shiftY;
        glBegin(GL_LINES);
        glVertex2f(secEnd, yPos-2);
        glVertex2f(secEnd, yPos+2);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(firEnd, yPos-2);
        glVertex2f(firEnd, yPos+2);
        glEnd();
        renderText (firEnd,yPos+15 , 0, QString::number(-max/2,'f',2),f);
        renderText (secEnd,yPos+15 , 0, QString::number(max/2,'f',2),f);
    }
    else if(_ratio < 1.0 || (_xscalingF < 1.0&&_isSample))
    {
        float secEnd = (_data.size()-1 -_xmin)*_xscaleF;
        float firEnd = ((-_xmin)*_xscaleF);
        float center = ((_data.size()-1 )/2 -_xmin)*_xscaleF + (1-_ratio) * (double)(this->width()-_xboarder)/2;
        double yPos = (_Ymax-_Ymin)*_yscaleF*_yscalingF + _shiftY  ;
        int k = 0;
        float space = (secEnd - firEnd)/2;
        while((center + k * space)* _xscalingF + _shiftX <= _width)
        {
            glBegin(GL_LINES);
            glVertex2f(center + k * space, yPos-2/_yscalingF);
            glVertex2f(center + k * space , yPos+2/_yscalingF);
            glEnd();
            renderText (center + k * space ,yPos+15/_yscalingF , 0, QString::number(k));
            k++;
        }
        k = -1;
        while((center + k * space)* _xscalingF + _shiftX >= _xboarder)
        {
            glBegin(GL_LINES);
            glVertex2f(center + k * space, yPos-2/_yscalingF);
            glVertex2f(center + k * space , yPos+2/_yscalingF);
            glEnd();
            renderText (center + k * space ,yPos+15/_yscalingF , 0, QString::number(k));
            k--;
        }
    }
}

void GLSpectrum::updateSampleRate(int sampleRate)
{
    _sampleRate = sampleRate;
}

void GLSpectrum::setScalingY(double scale)
{
    double oldScale = _yscalingF;
      if(scale == oldScale)
      {
        return;
      }
      _yscalingF = scale;

      if(scale < 1.0)
      {
        _yscalingF = 1.0;
      }

      double ycenter = (_height-_shiftY)/_yscaleF/oldScale;
      _shiftY -= qRound(ycenter*(_yscalingF-oldScale)*_yscaleF);

     // if(_shiftY < _Ymax*_yscaleF*(1-_yscalingF))
      {
        _shiftY = _Ymax*_yscaleF*(1-_yscalingF);
      }
    //  if(_yscalingF==1.0 || _shiftY > 0)
      {
     //   _shiftY=0;
      }
      updateGL();
}

void GLSpectrum::vIncrease()
{
    setScalingY(_yscalingF*(1.0+_scalingFactorY));
}

void GLSpectrum::vDecrease()
{
    setScalingY(_yscalingF*(1.0-_scalingFactorY));
}
