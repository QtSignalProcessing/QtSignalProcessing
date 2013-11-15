#include "glbase.h"

GLBase::GLBase(float *buf,int num,float time,bool sample,int sr,QWidget *parent)
    : QGLWidget(QGLFormat(QGL::NoSampleBuffers), parent),
      width(450), height(280), downSample(1),vIn(1),vDe(0),hIn(1),hshift(0),vshift(0),
      bits(0),L(1),currentMin(0),currentMax(time),sampleOrNot(sample),sr(sr),
      time(time),number(num),currentNumItems(num),data(buf),plotData(NULL),orgShift(0),xLabels(NULL),xCor(NULL),
      xShiftW1(30),xFactor2(0.1),replace(false)
{
    this->setWindowTitle("OpenGL widget");
    this->resize(width, height);
    max=getMax1(buf,num);
    xlabel = new AxisLabel(0, time);
    xScaleW1=number/(width-xShiftW1);
    yScaleW1=(float)(height-height/9)/1.5;
    yShiftW1=(height+height/10)/2;
    yMaxcord = max*(yScaleW1)+yShiftW1;
    yMincord = 2 * yShiftW1 - yMaxcord;
    ylabel=new AxisLabel(floor(-max), ceil(max));
    ylabel->setMaxNumSteps(8);
    computeCurrent();
    computeYCurrent();
}

GLBase::GLBase(float *buf,int num,QWidget *parent)
    : QGLWidget(QGLFormat(QGL::NoSampleBuffers), parent),
         width(450), height(280),downSample(1),vIn(1),vDe(0),hIn(1),hshift(0),vshift(0),bits(0),L(1)
    ,number(num),currentNumItems(number),data(buf),plotData(NULL),max(0),orgShift(0)
    ,xLabels(NULL),xCor(NULL),yLabels(NULL),yCor(NULL),xShiftW1(30),xFactor2(0.1)
{
    this->setWindowTitle("OpenGL widget");
    this->resize(width, height);
    xScaleW1=number/(width-xShiftW1);
    yScaleW1=(float)(height-height/9)/1.5;
    yShiftW1=(height+height/10)/2;
    replace=false;
}

void GLBase::resetH()
{
    this->hshift=0;
    hIn=1;
    computeCurrent();
    xFactor2=0.1;
    updateGL();
}

void GLBase::resetCommon()
{
    downSample = 1;
    vIn = 1;
    hIn = 1;
    hshift = 0;
    vshift = 0;
    L = 1;
    orgShift = 0;
    xFactor2 = 0.1;
    bits = 0;
    plotData = NULL;

}


float GLBase::getMax1(float *a,int num)
{
    float max=a[0];
    for(int i=1;i<num;i++)
        if(max<a[i])
            max=a[i];
    max += 0.3*max;
    return (max);
}

void GLBase::computeCurrent()
{
    int index1 = (-((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+hshift)*xScaleW1*hIn;
    currentMin = (float)index1 / number * time;
    int index2 = ( width - xShiftW1 - ((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)) + hshift ) * xScaleW1*hIn;
    currentMax = (float)index2 / number * time;
    xlabel->setCurrent(currentMin, currentMax);
}
void GLBase::computeYCurrent()
{
    currentYMax = (yMaxcord -vshift - yShiftW1) / ( yScaleW1 * vIn );
    currentYmin = (yMincord -vshift - yShiftW1) / ( yScaleW1 * vIn );
    ylabel->setCurrent(currentYmin,currentYMax);
}

void GLBase::nonIntSr(QString s)
{
   sampleRate=s.toInt();
   int sr=number/time;
   int cdG=gcd(sampleRate,sr);
   L=sampleRate/cdG;
   downSample=sr/cdG;
   if(sampleRate==sr)
   {
       downSample=1;L=1;
       currentNumItems=number;
   }
   updateGL();
}

QSize GLBase::minimumSizeHint() const
{
    return QSize(20, 20);
}

QSize GLBase::sizeHint() const
{
    return QSize(width, height);
}

void GLBase::vIncrease()
{
    vIn=vIn+0.1;
    computeYCurrent();
    updateGL();
}

void GLBase::vDecrease()
{
    if(vIn==1)
        return;
    vIn-=0.1;
    computeYCurrent();
    updateGL();
}

void GLBase::hIncrease()
{
    if(hIn>xFactor2)
    {
        hIn=hIn-xFactor2;
        computeCurrent();
        setHshift(orgShift);
    }
    else{
        xFactor2/=2;
        hIncrease();
    }
}

void GLBase::hDecrease()
{
    if(hIn>=0.95)
    {
        hIn=1;
        return;

    }else if(xFactor2<0.1)
    {
        xFactor2*=2;
        hIn+=xFactor2;
        computeCurrent();
        setHshift(orgShift);
    }else{
        hIn+=xFactor2;
        computeCurrent();
        setHshift(orgShift);
    }
}

void GLBase::setHshift(double shift)
{
   double len=(double)(number-1)/(xScaleW1*hIn)+(width/2-xShiftW1/2-(number/2)/(xScaleW1*hIn));
   this->hshift=shift/50.0*(len-width+xShiftW1);
   orgShift=shift;
   updateGL();
}

void GLBase::setVshift(int shift)
{
    int len=(yScaleW1*vIn)*max;
    this->vshift=shift/100.0*(len-height/2);
    updateGL();
}

void GLBase::resetV()
{
    this->vshift=0;
    vIn=1;
    computeYCurrent();
    updateGL();
}

void GLBase::setSampleRate(int sr)
{
    this->downSample=sr;
    updateGL();
}
void GLBase::setBits(int bits)
{
    this->bits=bits;
    updateGL();
}

int GLBase::gcd(int v1,int v2)
{
    while(v2)
    {
        int temp=v2;
        v2=v1%v2;
        v1=temp;
    }
    return v1;
}
void GLBase::initializeGL()
{
    glClearColor(1,1,1,1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    //project to plane for 2D drawing
    glOrtho(0, width, 0, height, 0, 128);
    glViewport(0,0, width, height);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}
void GLBase::resizeGL(int width, int height)
{
    this->width = width;
    this->height = height;
    xScaleW1=currentNumItems/(width-xShiftW1);
    yScaleW1=(float)(height-height/9)/1.5;
    yShiftW1=(height+height/10)/2;
    setHshift(orgShift);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0, 128);
    glViewport(0,0, width, height);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

}

void GLBase::setFacors(int orgShift)
{
    this->orgShift=orgShift;
}

void GLBase::setL(int L,int currentNumItems,int downSample,bool replace)
{
    this->L=L;
    this->currentNumItems=currentNumItems;
    this->downSample = downSample;
    this->replace = replace;
}

float GLBase::getMax1(double *a,int num)
{
    float max=a[0];
    for(int i=1;i<num-1;i++)
        if(max<a[i]){
            max=a[i];
        }
    return max;
}

void GLBase::resetDataAndOther(float *data, int num, float time, float max)
{
    this->data= data;
    this->number = num;
    this->time = time;
    this->max = max;
    currentNumItems = number;
}

float GLBase::getDist(float x1, float y1,float x2,float y2)
{
    float a = pow((x1-x2),2);
    float b = pow((y1-y2),2);
    return sqrt(a+b);
}

void GLBase::updateLabel(double max, double min)
{
     xlabel->updateMaxMin(max,min);
}

void GLBase::intToDouble(int b)
{
   setHshift(b);
}
