#include "glwidgetnew.h"
#include "utilities.h"

//#include <QGLWidget> qt4
#include <QOpenGLWidget>
#include <QDebug>
#include <QWheelEvent>
#include <QApplication>
#include <QSize>
#include <QPainter>
#include <QPaintEngine>

#include <math.h>


GLWidgetnew::GLWidgetnew(const QVector<float>&data,float time,bool isSample,QWidget* parent)
    : QOpenGLWidget(parent),
      _shiftX(0),
      _shiftY(0),
      _shiftXstart(0),
      _shiftYstart(0),
      _mousePressed(false),
      //_locked(false),
      _linked(true),
      _yscalingF(1.0),
      _xscalingF(1.0),
      _scalingFactorX(0.25),
      _scalingFactorY(0.25),
      _xscaleF(1.0),
      _yscaleF(1.0),
      _xmin(),
      _xboarder(this->width()/10),
      _yboarder(this->height()/18),
      _Xmax(10),
      _Xmin(0),
      _Ymax(200),
      _Ymin(0),
      _xGrid(250),
      _yGrid(50),
      _data(data),
      _step(1.0),
      _height(280),
      _width(450),
      _isSample(isSample),
      _stickon(false),
      _time(time),
      _offset(1)
{
    this->resize(_width, _height);
    setMouseTracking(true);
    _Xmax = _data.size();
    _Ymax = getMax()*10;
    _Ymin = -getMax()*10;

}

void GLWidgetnew::paintGL()
{
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 0.0);
    if(_isSample)
        glColor3f(0,0,1);
    else
        glColor3f(0,0,0);

    //Scalefactor
   // double oldxscaleF = _xscaleF;
   // double oldyscaleF = _yscaleF;

    _xscaleF = (double)(this->width()-_xboarder)/(double)(_Xmax-_Xmin);
    _yscaleF = (double)(this->height()-_yboarder)/(double)(_Ymax-_Ymin);
    //set new max/min values for free space at the widget borders
    _xmin = _Xmin - _xboarder/_xscaleF;

  //  _shiftX /= oldxscaleF/_xscaleF;
   // _shiftY /= oldyscaleF/_yscaleF;
    drawAxis();
    glTranslatef(float(_shiftX),float(_shiftY),0);
    glScalef(_xscalingF,_yscalingF,0);
  //  yLabel();
   // xLabel();
    if(!_isSample)
    {
        glBegin(GL_LINE_STRIP);

        for( int i = 0; i < _data.size(); i += 1)
        {
            if( (i-_xmin)*_xscaleF  * _xscalingF + _shiftX >= _xboarder &&  (i-_xmin)*_xscaleF  * _xscalingF + _shiftX < _width)
                glVertex2f( (i-_xmin)*_xscaleF  , (_Ymax-_data[i]*10)*_yscaleF );
        }
        glEnd();
    }
    else
    {
        float cx;
        for( int i = 0; i < _data.size(); i+= _offset)
        {
            cx = (i-_xmin)*_xscaleF;
            float cy = (_Ymax-_data[i]*10)*_yscaleF;
            float newCord = cx * _xscalingF + _shiftX ;
            if( newCord >= _xboarder && newCord <= _width )
            {
                glBegin(GL_LINE_LOOP);
                for(int ii = 0; ii < 8; ii++)
                {
                    float theta = 2.0f * 3.1415926f * float(ii) / float(8);//get the current angle
                    float r = 5;
                    float x = r * cosf(theta)/(_xscalingF*(1.0+_scalingFactorX));//calculate the x component
                    float y = r * sinf(theta);//calculate the y component
                    glVertex2f(x + cx, y + cy);//output vertex
                }
                glEnd();
                if(_stickon)
                {
                    glBegin(GL_LINES);
                    glVertex2f(cx,cy);
                    glVertex2f(cx,(this->height()-_yboarder)/2);
                    glEnd();
                }
            }
        }
    }
    yLabel();
    xLabel();
    glPopMatrix();


}


void GLWidgetnew::wheelEvent(QWheelEvent *event)
{

    if(event->angleDelta().y() > 0)
    {
        if(Qt::AltModifier != QApplication::keyboardModifiers())
        {
            setScalingX(_xscalingF*(1.0+_scalingFactorX));
        }
    }
    else
    {
        if(Qt::AltModifier != QApplication::keyboardModifiers())
        {
            setScalingX(_xscalingF*(1.0-_scalingFactorX));
        }
    }
}

void GLWidgetnew::initializeGL()
{
    glClearColor(1,1,1,1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    //project to plane for 2D drawing
    glOrtho(0, _width, _height, 0, 0, 128);
    glViewport(0,0, _width, _height);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

}
void GLWidgetnew::resizeGL(int width, int height)
{
    _width = width;
    _height = height;
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, _width, _height, 0, 0, 128);
    glViewport(0,0, _width, _height);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}

float GLWidgetnew::getMax()
{
    float max = 0;
    for(int i = 0; i < _data.size(); i++)
    {
        if(fabs(_data[i]) > max)
            max = _data[i];
    }
    if( max < 1.0 )
    {
        max = 1.0;
        return max;
    }
    return max + 0.1;
}

float GLWidgetnew::getMin()
{
    float min = 10000;
    for(int i = 0; i < _data.size(); i++)
    {
        if(_data[i] < min)
            min = _data[i];
        if(min > -1.0)
        {
            min = -1.0;
            return min;
        }
    }
    return min;
}

void GLWidgetnew::mouseMoveEvent(QMouseEvent* event)
{
    this->setCursor(Qt::ArrowCursor);
    _xMouse = event->pos().x() + _xboarder;
    _yMouse = event->pos().y();

    if(_xscalingF < 1.0)
        return;

    if(_mousePressed  && (event->buttons() & Qt::LeftButton)  )
    {
        if(_xscalingF!=1.0)
        {
            this->setCursor(Qt::ClosedHandCursor);
        }
        _shiftX = _shiftXstart + _xMouse - _mousePressedX - _xboarder;
        _shiftY = _shiftYstart + _yMouse - _mousePressedY;

        if(_shiftX > 0)
        {
            _shiftX = 0;
            _shiftXstart = _shiftX;
            _mousePressedX = _xMouse;
        }
        if(_shiftX < double(_Xmax)*_xscaleF*(1-_xscalingF))
        {
            _shiftX = double(_Xmax)*_xscaleF*(1-_xscalingF);
            _shiftXstart = _shiftX;
            _mousePressedX = _xMouse;
        }
        if(_linked)
        {
            emit xAxisShifted(_shiftX);
            emit xAxisScaled(_xscalingF);
        }
    }
    update();
}

void GLWidgetnew::mousePressEvent(QMouseEvent* event)
{
    _mousePressedX = event->pos().x();
    _mousePressedY = event->pos().y();
    _mousePressed = true;
    _shiftXstart = _shiftX;
    _shiftYstart = _shiftY;
}

void GLWidgetnew::mouseReleaseEvent(QMouseEvent*)
{
    _mousePressed = false;
    if(_xscalingF != 1.0)
        this->setCursor(Qt::OpenHandCursor);
    else
        this->setCursor(Qt::ArrowCursor);
}

void GLWidgetnew::setScalingX(double scale)
{
    double oldScale = _xscalingF;
    if(scale == oldScale)
        return;
    _xscalingF = scale;
    if(scale < 1.0)
        _xscalingF = 1.0;

    double xcenter = (_xMouse-_shiftX+_xmin*_xscaleF)/_xscaleF/oldScale;
    _shiftX -= qRound(xcenter*(_xscalingF-oldScale)*_xscaleF);
    if(_shiftX < _Xmax*_xscaleF*(1-_xscalingF))
        _shiftX = _Xmax*_xscaleF*(1-_xscalingF);

    if(_shiftX > 0 || _xscalingF==1.0)
        _shiftX = 0;

    update();
    if(_linked)
    {
        emit xAxisScaled(_xscalingF);
        emit xAxisShifted(_shiftX);
    }
}

void GLWidgetnew::DrawCircle(float cx, float cy, float r, int num_segments)
{
    glBegin(GL_LINE_LOOP);
    for(int ii = 0; ii < num_segments; ii++)
    {
       float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle
       float x = r * cosf(theta);//calculate the x component
       float y = r * sinf(theta);//calculate the y component
       glVertex2f(x + cx, y + cy);//output vertex
    }
    glEnd();
}

float GLWidgetnew::getDist(float x1, float y1,float x2,float y2)
{
    float a = pow((x1-x2),2);
    float b = pow((y1-y2),2);
    return sqrt(a+b);
}

void GLWidgetnew::drawAxis()
{
    //draw axis
    glBegin(GL_LINES);
    glVertex2f(0,(_Ymax-_Ymin)*_yscaleF); //x-axis
    glVertex2f(this->width(), (_Ymax-_Ymin)*_yscaleF);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-_xmin*_xscaleF,0); //y-axis
    glVertex2f(-_xmin*_xscaleF, this->height()-10);
    glEnd();

    //draw x-arrow
    glBegin(GL_LINES);
    glVertex2f(this->width()-8, (_Ymax-_Ymin)*_yscaleF+3);
    glVertex2f( this->width(), (_Ymax-_Ymin)*_yscaleF);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(this->width()-8, (_Ymax-_Ymin)*_yscaleF-3);
    glVertex2f(this->width(), (_Ymax-_Ymin)*_yscaleF);
    glEnd();

    //draw y-arrow
    glBegin(GL_LINES);
    glVertex2f(-_xmin*_xscaleF, 0);
    glVertex2f(-_xmin*_xscaleF+3, 8);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-_xmin*_xscaleF, 0);
    glVertex2f( -_xmin*_xscaleF-3, 8);
    glEnd();
}

void GLWidgetnew::setData(const QList<float>&data)
{
    _data = data;
    update();
}

void GLWidgetnew::setSNR(double snr)
{
    emit newSNR(snr);
}

void GLWidgetnew::setStickOn(bool i)
{
    _stickon = i;
    update();
}

void GLWidgetnew::resetH()
{
    setScalingX(1.0);
    _shiftX = 0;
    _shiftY = 0;
    _yscalingF = 1.0;
    update();
}

void GLWidgetnew::yLabel(int offset)
{
    uint n = height()/20*_yscalingF;
    if (n < 1) { n = 1; }
    double S[]={1,2,5};
    uint i = 0;
    uint grid = 1;
    while(!((int)(n*grid)>(_Ymax-_Ymin)))
    {
        ++i;
        grid = pow(10,i/3)*S[i%3];
    }
    _yGrid = grid;

    glScalef(1.0/_xscalingF,1.0/_yscalingF,0);



    //draw y-grid

    for (int i = _Ymin; i <= _Ymax; i = i + _yGrid)
    {
        double yPos = (_Ymax-i)*_yscaleF*_yscalingF;
        double xPos = -_xmin*_xscaleF-_shiftX;


        if(yPos < (_Ymax-_Ymin)*_yscaleF-_shiftY+1)
        {

            glBegin(GL_LINES);
            glVertex2f(xPos-4, yPos);
            glVertex2f(xPos+4, yPos);
            glEnd();
            qt_save_gl_state();
            renderText(xPos - offset,yPos+5 , QString::number(i/10.0));
            qt_restore_gl_state();

        }
    }
    glScalef(_xscalingF,_yscalingF,0);
  //  QPainter p(this);

}

void GLWidgetnew::xLabel()
{
    int index1 = (_xboarder - _shiftX) / _xscalingF / _xscaleF + _xmin;
    double currentMin = (float)index1 / _data.size() * _time;
    int index2 = (_width - _shiftX) / _xscalingF / _xscaleF + _xmin;
    double currentMax = (float)index2 / _data.size() * _time;
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
    int num = _time / _step;
    _xGrid = floor(_Xmax / num);
    if(_xGrid < 1)
    {
        if(_xGrid < 0)
            return;
        _xGrid = 1;
    }
    int digit = round(fabs(log10 ( _step)-1));
    if(digit < 0)
        digit = 6;
    glScalef(1.0/_xscalingF,1.0/_yscalingF,0);

    int k = 0;
    for (int i = 0; i < _Xmax; i += _xGrid)
    {
      double xPos =  i*_xscaleF*_xscalingF-_xmin*_xscaleF;
      double yPos = (_Ymax-_Ymin)*_yscaleF-_shiftY;
   if(((xPos > -_shiftX - _xmin*_xscaleF + 10) &&(xPos < _Xmax*_xscaleF -_shiftX - _xmin*_xscaleF + 10 )) || (i == 0 && _shiftX == 0)  )
      {
        glBegin(GL_LINES);
        glVertex2f(xPos, yPos-2);
        glVertex2f(xPos, yPos+2);
        glEnd();
      //  if(((xPos > -_shiftX - _xmin*_xscaleF + 10) &&(xPos < _Xmax*_xscaleF -_shiftX - _xmin*_xscaleF + 10 )) || (i == 0 && _shiftX == 0)  )

        //renderText (xPos - 10,yPos+15 , QString::number(k*_step,'f',digit));
        qt_save_gl_state();
        renderText (xPos - 10,yPos+15 , QString::number(k*_step,'f',digit));
        qt_restore_gl_state();

        k++;
      }
    }
    glScalef(_xscalingF,_yscalingF,0);
}

void GLWidgetnew::setLinkedScale(double scale)
{
    _xscalingF = scale;
    update();
}

void GLWidgetnew::setLinkedShift(double shift)
{
    _shiftX = shift;
    update();
}

void GLWidgetnew::updateMax()
{
    _Xmax = _data.size();
    _Ymax = getMax()*10;
    _Ymin = -getMax()*10;
    _Xmin = 0;
    update();
}

void GLWidgetnew::setOffset(int offset)
{
    _offset = offset;
    update();
}

QSize GLWidgetnew::minimumSizeHint() const
{
    return QSize(20, 20);
}

QSize GLWidgetnew::sizeHint() const
{
    return QSize(_width, _height);
}

bool GLWidgetnew::getIsSample()
{
    return _isSample;
}

void GLWidgetnew::setTime(float time)
{
    _time = time;
}

void GLWidgetnew::renderText(double x, double y, const QString text)
{
    GLdouble textPosX = x, textPosY = y;
    // Retrieve last OpenGL color to use as a font color
    GLdouble glColor[4];
    glGetDoublev(GL_CURRENT_COLOR, glColor);
    QColor fontColor = QColor(glColor[0]*255, glColor[1]*255, glColor[2]*255, glColor[3]*255);
    // Render text
    QPainter painter(this);
    painter.translate(float(_shiftX),float(_shiftY)); //This is for my own mouse event (scaling)
    painter.setPen(fontColor);
    QFont f;
    f.setPixelSize(10);
    painter.setFont(f);
    painter.drawText(textPosX, textPosY, text);
    painter.end();
}


void GLWidgetnew::qt_save_gl_state()
{
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glShadeModel(GL_FLAT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void GLWidgetnew::qt_restore_gl_state()
{
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    glPopClientAttrib();
}
