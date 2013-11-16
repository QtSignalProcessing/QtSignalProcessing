#include "glospectrum.h"

#include "axislabel.h"
#include "iir.h"
#include "cheby1.h"

#include "fftw3.h"

GLOSpectrum::GLOSpectrum(float *buf,int num,bool discrete,int sr,QWidget *parent)
    : GLBase(buf,num,parent),
     factor(1),showRange(false),filterData(NULL),
     NyqFreq(1),order(2),showFilter(false),currentIndex(0),ripple(0.5),
     conFirst(false),disFirst(false),show(false),yTimeOfScale(0),discrete(discrete),sr((float)sr/1000),
     xMin(0),xMax(2),showOnePeriod(false),numofP(0),
     totalValue(NULL),totalValue1(NULL),amplitude(NULL),amplitude1(NULL),amplitude2(NULL),showAliasing(false),getAlias(false),
     timeOfScale(0),banned(false),_currentYmax(0),yl()
{
   xlabel = new AxisLabel(xMin, xMax);
   xlabel->setMaxNumSteps(4);
   currentMin=xMin;
   currentMax=xMax;
}

GLOSpectrum::~GLOSpectrum()
{}

void GLOSpectrum::resetV()
{
   this->vshift=0;
   vIn=1;
   yTimeOfScale=0;
   updateGL();
}

void GLOSpectrum::vIncrease()
{
   if(banned)
      return;
   vIn=vIn+0.5;
   yTimeOfScale++;
   updateGL();
}

void GLOSpectrum::vDecrease()
{
   if(vIn==1||banned)
       return;
   vIn-=0.5;
   yTimeOfScale--;
   updateGL();
}

void GLOSpectrum::setVshift(int shift)
{
    if(vIn!=1)
    {
      int len=(yScales*vIn)*_max;
      this->vshift=(float)shift/100.0*(len-height);
    }
    updateGL();
}

void GLOSpectrum::hDecrease()
{
    if((numofP>2&&timeOfScale<=0))
        return;
    if(!discrete&&banned&&timeOfScale<=0)
        return;
     if(xFactor2<0.1)
    {
        xFactor2*=2;
        hIn+=xFactor2;
        timeOfScale--;
        computeCurrent();
        setHshift(orgShift);
    }
    else
    {
       hIn+=xFactor2;
       timeOfScale--;
       setHshift(orgShift);
    }
}

void GLOSpectrum::resetH()
{
    this->hshift=0;
    hIn=1;
    xFactor2=0.1;
    orgShift = 0;
    setHshift(orgShift);
}

void GLOSpectrum::nonIntSr(QString s)
{
   int num=s.toInt();
   int cdG=gcd(num,sr*1000);
   L=num/cdG;
   downSample=sr*1000/cdG;
   if(num==sr*1000)
   {
       downSample=1;L=1;
       currentNumItems=number;
   }
   factor=s.toFloat()/sr/1000;
   updateGL();
}

void GLOSpectrum::showSampleRange(int state)
{
    if(state==0)
    {
        showRange=false;
    }else{
        showRange=true;
    }
    updateGL();
}

void GLOSpectrum::butterOrCheby(int index)
{
    if((index==-1)||(index==0))
        butter=true;
    else butter=false;
}

void GLOSpectrum::setOrder(QString order)
{
    this->order=order.toInt();
    if(this->order > 15)
    {
        if(filterData!=NULL)
            free(filterData);
        filterData=(float*)malloc(number*sizeof(float));
        for(int i = 0;i < number; i++)
            filterData[i] = data[i];
        emit OrderTooHigh(true);
        return;
    }
}

void GLOSpectrum::Filter()
{
    if(ripple <= 0 || order >15)
        return;
    showFilter=true;
    if(filterData!=NULL)
        free(filterData);
    filterData=(float*)malloc(number*sizeof(float));
    for(int i=0;i<number;i++)
        filterData[i]=0;
    double* b1=(double*)malloc((order+1)*sizeof(double));
    double* a1=(double*)malloc((order+1)*sizeof(double));
    if(butter)
    {
        IIR* iir=new IIR(0,order,factor,0,0);
        for(int i=0;i<=order;i++)
        {
            b1[i]=iir->b[i];
            a1[i]=iir->a[i];
        }
        delete iir;
    }
    else
    {
        cheby1* d=new cheby1(1,order,ripple,sr*1000,sr*1000*(factor)/2,0);
        for(int i=0;i<=order;i++)
        {
            b1[i]=d->pp[i];
            a1[i]=d->aa[i];
        }
        delete d;
    }
    int i,j;
    filterData[0]=b1[0]*data[0];
    for (i=1;i<order+1;i++)
    {
        filterData[i]=0.0;
        for (j=0;j<i+1;j++)
                filterData[i]=filterData[i]+b1[j]*data[i-j];
        for (j=0;j<i;j++)
                filterData[i]=filterData[i]-a1[j+1]*filterData[i-j-1];
    }
    /* end of initial part */
    for (i=order+1;i<number+1;i++)
    {
        filterData[i]=0.0;
        for (j=0;j<order+1;j++)
        filterData[i]=filterData[i]+b1[j]*data[i-j];
        for (j=0;j<order;j++)
        filterData[i]=filterData[i]-a1[j+1]*filterData[i-j-1];
    }
    free(a1);
    free(b1);
    if(fabs(getMax1(filterData,number))>3)
    {
        for(int i = 0;i < number; i++)
            filterData[i] = data[i];
        emit OrderTooHigh(true);
    }
    else
        emit OrderTooHigh(false);
    updateGL();
}

void GLOSpectrum::setNyqFreq(QString s)
{
    NyqFreq=s.toInt()/sr/1000/2;
}

void GLOSpectrum::setToN()
{
    factor=NyqFreq;
    Filter();
}

void GLOSpectrum::show2F(bool state)
{
    this->show2Spec=state;
    if(show2Spec==false)
    {
        conFirst=false;
        disFirst=false;
        updateGL();
    }
}

void GLOSpectrum::conDis()
{
    conFirst=true;
    disFirst=false;
    updateGL();
}

void GLOSpectrum::disCon()
{
    conFirst=false;
    disFirst=true;
    updateGL();
}

void GLOSpectrum::paintGL()
{
    computeCurrent();
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT);
    if(!discrete)
        glColor3f(0,0,0);
    else if(discrete&&!showAliasing)
        glColor3f(0,0,1);
    else
        glColor3f(255,0,0);
    glLineWidth(5);
    double  *o=(double*)malloc(sizeof(double)*number);
    for(int i=0;i<number;i++)
        o[i]=data[i];
    currentNumItems=number*(float)L/downSample;
    if(showRange&&discrete)
    {
        float xCor1;
        float xCor2;
        xCor1=(float)(xShiftW1+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-hshift);
        xCor2=(float)(currentNumItems/(xScaleW1*hIn)+xShiftW1+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-hshift);
        glBegin(GL_POLYGON);
        glColor3f(0,1,0);
        glVertex2f(xCor1, yShiftW1);
        glVertex2f(xCor2, yShiftW1);
        glVertex2f(xCor2, height);
        glVertex2f(xCor1, height);
        glEnd();
    }
    xlabel->autoScale();
    currentNumItems=number*(float)L/downSample;
    if(totalValue!=NULL)
    {
        free(totalValue);
    }
    totalValue=(double*)malloc(sizeof(double)*(currentNumItems));
    if(amplitude!=NULL)
    {
        free(amplitude);
    }
    amplitude=getAmplitude(currentNumItems,o);
    for(int i=0;i<currentNumItems/2;i++)
    {
        totalValue[currentNumItems/2-i]=amplitude[i];
    }
    for(int i=0;i<currentNumItems/2;i++)
    {
        totalValue[i+currentNumItems/2]=amplitude[i];
    }
    if(currentNumItems == number)
    {
        _max=getMax1(  amplitude,currentNumItems/2/downSample);
        yScales =  (float)(height-height/9)/_max;
        _currentYmax = _max;
    }
    glColor3f(1,0,0);
    glLineWidth(1);
    float ptDist = 0;
    int previousIndex = 0;
    if(disFirst)
    {
        show=true;
        if(amplitude1!=NULL)
            free(amplitude1);
        amplitude1=getAmplitude(number,o);
        show=false;
        glBegin(GL_LINE_STRIP);
        xScaleW1=number/(width-xShiftW1);
        yShiftW1=height/10;
         //the following 2 for loops plot the spectrum of the original signal
        for (int x = 0; x <=number/2; x++)
        {
            if(((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[x]*yScales*vIn+yShiftW1-vshift>height/11))
            {
                float xcor = (x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn));
                if(fabs(getDist(ptDist,amplitude1[ previousIndex ]*(yScales*vIn)+yShiftW1+vshift,xcor,amplitude1[x]*(yScales*vIn)+yShiftW1+vshift)) >=1)
                {
                    glVertex2f((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), amplitude1[x]*yScales*vIn+yShiftW1-vshift);
                    ptDist = xcor;
                    previousIndex = x;
                }
            }
            if(((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[x]*yScales*vIn+yShiftW1-vshift<=height/11))
                glVertex2f((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), height/11);
         }
        glEnd();
        ptDist = 0;
        previousIndex = 0;
        glBegin(GL_LINE_STRIP);
        for (int x =  number/2; x >=0; x--)
            {
                if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift>height/11))
                {
                    float xcor = (x)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn));
                    if(fabs(getDist(ptDist,amplitude1[ number/2-1-previousIndex ]*(yScales*vIn)+yShiftW1+vshift,xcor,amplitude1[number/2-1-x]*(yScales*vIn)+yShiftW1+vshift)) >=1)
                    {
                        glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift);
                        ptDist = xcor;
                        previousIndex = x;
                    }
                }
                if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift<=height/11))
                    glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), height/11);
             }
        conFirst=false;
        glEnd();
    }
    if(discrete)
        for(int k=-currentNumItems/2;k<=currentNumItems/2;k++)      
             o[k+currentNumItems/2]=(float)2*(k)/currentNumItems;
    if(!discrete)
        for(int k=-currentNumItems/2;k<currentNumItems/2;k++)     
             o[k+currentNumItems/2]=(float)k/currentNumItems*sr;
    glLineWidth(1.0f);
    if(!discrete)
        glColor3f(0,0,0);
    else if(discrete&&!showAliasing)
        glColor3f(0,0,1);
    else
        glColor3f(255,0,0);
    {
        glBegin(GL_LINE_STRIP);
        xScaleW1=number/(width-xShiftW1);
        yShiftW1=height/10;
         //the following 2 for loops plot the spectrum of the original signal
        int x;
        ptDist = 0;
        previousIndex = 0;
        for ( x = 0; x <=currentNumItems/2; x++)
        {
            if(((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude[x]*yScales*vIn+yShiftW1-vshift>height/11))
            {
                float xcor = (x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
                if(fabs(getDist(ptDist,amplitude[ previousIndex ]*(yScales*vIn)+yShiftW1+vshift,xcor,amplitude[x]*(yScales*vIn)+yShiftW1+vshift)) >=1)
                {
                    glVertex2f((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), amplitude[x]*yScales*vIn+yShiftW1-vshift);
                    ptDist = xcor;
                    previousIndex = x;
                }
            }
            if(((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude[x]*yScales*vIn+yShiftW1-vshift<=height/11))
                glVertex2f((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), height/11);
         }
        if(discrete)
        {
            float xCord=(x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
            int i=0,j=0;
            previousIndex = 0;
            ptDist = 0;
            if(!showOnePeriod)
            {
                while(width-(i/(xScaleW1*hIn)+xCord)>0)
                {
                    if((i/(xScaleW1*hIn)+xCord<width)&&(totalValue[j]*yScales*vIn+yShiftW1-vshift>height/11))
                    {
                        float xcor = i/(xScaleW1*hIn)+xCord;
                        if(fabs(getDist(ptDist,totalValue[ previousIndex ]*(yScales*vIn)+yShiftW1+vshift,xcor,totalValue[j]*(yScales*vIn)+yShiftW1+vshift)) >=1)
                        {
                            glVertex2f(i/(xScaleW1*hIn)+xCord, totalValue[j]*yScales*vIn+yShiftW1-vshift);
                            ptDist = xcor;
                            previousIndex = j;
                        }
                    }
                    if((i/(xScaleW1*hIn)+xCord>xShiftW1)&&(totalValue[j]*yScales*vIn+yShiftW1-vshift<=height/11))
                        glVertex2f(i/(xScaleW1*hIn)+xCord, height/11);
                   i++;
                   j++;
                   if(j>=currentNumItems-1)
                       j=0;
                }
            }
        }
        glEnd();
        previousIndex = 0;
        ptDist = 0;
        glBegin(GL_LINE_STRIP);
        for ( x =  currentNumItems/2; x >=0; x--)
            {
                float xCord=(x)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
                if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude[currentNumItems/2-x]*yScales*vIn+yShiftW1-vshift>height/11))
                {
                    if(fabs(getDist(ptDist,amplitude[currentNumItems/2- previousIndex ]*(yScales*vIn)+yShiftW1+vshift,xCord,amplitude[currentNumItems/2-x]*(yScales*vIn)+yShiftW1+vshift)) >=1)
                    {
                        glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), amplitude[currentNumItems/2-x]*yScales*vIn+yShiftW1-vshift);
                        ptDist = xCord;
                        previousIndex = x;
                    }
                }
                if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude[currentNumItems/2-x]*yScales*vIn+yShiftW1-vshift<=height/11))
                    glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), height/11);
             }
        if(discrete)
        {
            int i=0,j=0;
            float xCord=xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
            ptDist = 0;
            previousIndex = 0;
            if(!showOnePeriod)
            {
               while((xCord-i/(xScaleW1*hIn))>0)
                {
                   float xcord = xCord-(i/(xScaleW1*hIn));
                   if((xCord-(i/(xScaleW1*hIn))>xShiftW1)&&(totalValue[j]*yScales*vIn+yShiftW1-vshift>height/11))
                      {
                       if(fabs(getDist(ptDist,totalValue[ previousIndex ]*(yScales*vIn)+yShiftW1+vshift,xcord,totalValue[j]*(yScales*vIn)+yShiftW1+vshift)) >=1)
                        {
                           glVertex2f(xCord-i/(xScaleW1*hIn), totalValue[j]*yScales*vIn+yShiftW1-vshift);
                           ptDist = xcord;
                           previousIndex = j;
                        }
                      }
                   if((xCord-i/(xScaleW1*hIn)>xShiftW1)&&(totalValue[j]*yScales*vIn+yShiftW1-vshift<=height/11))
                       glVertex2f(xCord-i/(xScaleW1*hIn), height/11);
                   i++;
                   j++;
                   if(j>=currentNumItems-1)
                      j=0;
                 }
             }
        }
        glEnd();
        if(showAliasing && discrete)
        {
            glColor3f(0,0,1);
            getAlias = true;
            if(amplitude2!=NULL)
                free(amplitude2);
            double* tmp1= (double*)malloc(sizeof(double)*(number));
            for(int i = 0;i<number; i++)
                tmp1[i]= data[i];
            amplitude2 = getAmplitude(currentNumItems,tmp1);
            getAlias = false;
            free(tmp1);
            if(totalValue1 != NULL)
                free(totalValue1);
            totalValue1 = (double*)malloc(sizeof(double)*(currentNumItems));
            for(int i=0;i<currentNumItems/2;i++)
                totalValue1[currentNumItems/2-i]=amplitude2[i];
            for(int i=0;i<currentNumItems/2;i++)
                totalValue1[i+currentNumItems/2]=amplitude2[i];
            glBegin(GL_LINE_STRIP);
            int x;
            ptDist =0;
            previousIndex = 0;
            for ( x = 0; x <currentNumItems/2; x++)
            {
                float xcor = (x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
                if(((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude2[x]*yScales*factor*vIn+yShiftW1-vshift>height/11))
                {
                    if(fabs(getDist(ptDist,amplitude2[ previousIndex ]*(yScales*factor*vIn)+yShiftW1+vshift,xcor,amplitude2[x]*(yScales*factor*vIn)+yShiftW1-vshift)) >=1)
                    {
                        glVertex2f((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), amplitude2[x]*yScales*factor*vIn+yShiftW1-vshift);
                        ptDist = xcor;
                        previousIndex = x;
                    }
                }
                if(((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude2[x]*yScales*factor*vIn+yShiftW1-vshift<=height/11))
                    glVertex2f((x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), height/11);
             }
            if(discrete)
            {
                float xCord=(x+currentNumItems/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
                int i=0,j=0;
                ptDist = 0;
                previousIndex = 0;
                if(!showOnePeriod)
                {
                    while(width-(i/(xScaleW1*hIn)+xCord)>0)
                    {
                        float xcord = i/(xScaleW1*hIn)+xCord;
                        if((i/(xScaleW1*hIn)+xCord<width)&&(totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift>height/11))
                        {
                            if(fabs(getDist(ptDist,totalValue1[ previousIndex ]*(yScales*factor*vIn)+yShiftW1+vshift,xcord,totalValue1[j]*(yScales*factor*vIn)+yShiftW1+vshift)) >=1)
                            {
                                glVertex2f(i/(xScaleW1*hIn)+xCord, totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift);
                                ptDist = xcord;
                                previousIndex = j;
                            }
                        }
                        if((i/(xScaleW1*hIn)+xCord>xShiftW1)&&(totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift<=height/11))
                            glVertex2f(i/(xScaleW1*hIn)+xCord, height/11);
                       i++;
                       j++;
                       if(j>=currentNumItems-1)
                           j=0;
                    }
                }
            }
            glEnd();
            ptDist = 0;
            previousIndex = currentNumItems/2;
            glBegin(GL_LINE_STRIP);
            for ( x =  currentNumItems/2; x >=0; x--)
                {
                    float xcord = x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
                    if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude2[currentNumItems/2-x]*yScales*factor*vIn+yShiftW1-vshift>height/11))
                    {
                        if(getDist(ptDist,amplitude2[currentNumItems/2-previousIndex]*yScales*factor*vIn+yShiftW1-vshift,xcord,amplitude2[currentNumItems/2-x]*yScales*factor*vIn+yShiftW1-vshift) >= 1)
                        {
                            glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), amplitude2[currentNumItems/2-x]*yScales*factor*vIn+yShiftW1-vshift);
                            ptDist = xcord;
                            previousIndex = x;
                        }
                    }
                    if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude2[currentNumItems/2-x]*yScales*factor*vIn+yShiftW1-vshift<=height/11))
                        glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)), height/11);
                 }
            if(discrete)
            {
                int i=1,j=0;
                ptDist =0;
                previousIndex = 0;
                 float xCord=xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
                 if(!showOnePeriod)
                 {
                    while((xCord-i/(xScaleW1*hIn))>0)
                     {
                         float xcord = xCord-i/(xScaleW1*hIn);
                       if((xCord-(i/(xScaleW1*hIn))>xShiftW1)&&(totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift>height/11))
                         {
                           if(getDist(ptDist,totalValue1[previousIndex]*yScales*factor*vIn+yShiftW1-vshift,xcord,totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift) >= 1)
                           {
                           glVertex2f(xCord-i/(xScaleW1*hIn), totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift);
                            ptDist = xcord;
                            previousIndex = j;
                           }
                         }

                       if((xCord-i/(xScaleW1*hIn)>xShiftW1)&&(totalValue1[j]*yScales*factor*vIn+yShiftW1-vshift<=height/11))
                           glVertex2f(xCord-i/(xScaleW1*hIn), height/11);
                       i++;
                       j++;
                       if(j>=currentNumItems-1)
                          j=0;
                     }
                 }
            }
            glEnd();
            glColor3f(0,0,1);
        }
//end of showAliasing
       glBegin(GL_LINES);
       glVertex2f(0, height/11);
       glVertex2f(width, height/11 );
        glEnd();
       //y-axis
        glBegin(GL_LINES);
       glVertex2f(xShiftW1, 0);
       glVertex2f(xShiftW1, height);
       glEnd();
       numofP=number / currentNumItems;
       if(numofP >= 3)
           emit TooManyPeriods(true);
       else emit TooManyPeriods(false);
       if(hIn<=1&&(numofP<2))
           plotXAxis();
       else
           plotAxis();
       ygenerateAxis();
  }
    if(showRange)
    {
        if(!discrete)
        {
            glColor3f(255,0,0);
        }
        glBegin(GL_LINES);
        float xCor1;
        float xCor2;
        if(!discrete)
        {
            xCor1=currentNumItems/2/(xScaleW1)-hshift+xShiftW1-(currentNumItems/2)/(xScaleW1*hIn)*factor;
            xCor2=currentNumItems/2/(xScaleW1)-hshift+(currentNumItems/2)/(xScaleW1*hIn)*factor+xShiftW1;
        }
        else
        {
            xCor1=(float)(xShiftW1+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-hshift);
            xCor2=(float)(currentNumItems/(xScaleW1*hIn)+xShiftW1+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-hshift);
        }
        glVertex2f(xCor1,yShiftW1);
        glVertex2f(xCor1,height);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(xCor2,yShiftW1);
        glVertex2f(xCor2,height);
        glEnd();
    }
    if(conFirst&&(!show))
    {
        for(int i=0;i<number;i++)
            o[i]=data[i];
        glColor3f(1,0,0);
        glLineWidth(1);
        disFirst=false;
        show=true;
        amplitude1=getAmplitude(number,o);
        show=false;
        glBegin(GL_LINE_STRIP);
        xScaleW1=number/(width-xShiftW1);
        yScaleW1=(float)height/560;
        yShiftW1=height/10;
        ptDist = 0;
        previousIndex = 0;
        for (int x = 0; x <=number/2; x++)
        {
            float xcord = (x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn));
            if(((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[x]*yScales*vIn+yShiftW1-vshift>height/11))
            {
                if(getDist(ptDist,amplitude1[previousIndex]*yScales*vIn+yShiftW1-vshift,xcord,amplitude1[x]*yScales*vIn+yShiftW1-vshift) >= 1)
                {
                    glVertex2f((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), amplitude1[x]*yScales*vIn+yShiftW1-vshift);
                    ptDist = xcord;
                    previousIndex = x;
                }
            }
            if(((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[x]*yScales*vIn+yShiftW1-vshift<=height/11))
                glVertex2f((x+number/2)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), height/11);
         }
        glEnd();
        ptDist = 0;
        previousIndex = 0;
        glBegin(GL_LINE_STRIP);
        for (int x =  number/2; x >=0; x--)
            {
                float xcord = (x)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn));
                if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift>height/11))
                {
                    if(getDist(ptDist,amplitude1[number/2-1-previousIndex]*yScales*vIn+yShiftW1-vshift,xcord,amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift) >= 1)
                    {
                        glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift);
                        ptDist = xcord;
                        previousIndex = x;
                    }
                }
                if((x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))>xShiftW1)&&(amplitude1[number/2-1-x]*yScales*vIn+yShiftW1-vshift<=height/11))
                    glVertex2f(x/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)), height/11);
             }
        glEnd();
    }
    free(o);
    glPopMatrix();
    glPopAttrib();
}

double* GLOSpectrum::getAmplitude(int num,double *in)
{
    fftw_complex *out;
    fftw_plan p;
    double *orgIn=(double*)malloc(sizeof(double)*(num));
    double* in1;
    for(int j=0;j<num;j++)
        orgIn[j]=in[j];
    if((disFirst&&show)||(conFirst&&show))
    {
        if(disFirst)
            num=number;
        out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(num));
        p=fftw_plan_dft_r2c_1d(number,orgIn,out,FFTW_ESTIMATE);
        fftw_execute(p);
        double real,imag;
        amplitude1=(double*)malloc(sizeof(double)*(number/2));
        for(int j=0;j<number/2;j++)
        {
            real=out[j][0];
            imag=out[j][1];
            amplitude1[j]=sqrt(real*real+imag*imag);
        }
        free(orgIn);
        fftw_destroy_plan(p);
        fftw_free(out);
        show=false;
        return amplitude1;
    }
    if(showFilter)
    {
        if(fabs(factor-1.0)<10e-6)
        {
            for(int j=0;j<currentNumItems;j++)
                orgIn[j]=data[j];
        }
        else
        {
           for(int j=0;j<currentNumItems;j++)
                orgIn[j]=filterData[j];
        }
        out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(currentNumItems));
        p=fftw_plan_dft_r2c_1d(currentNumItems,orgIn,out,FFTW_ESTIMATE);
        fftw_execute(p);
        free(orgIn);
        double real,imag;
        amplitude=(double*)malloc(sizeof(double)*(currentNumItems/2));
        for(int j=0;j<currentNumItems/2;j++)
        {
            real=out[j][0];
            imag=out[j][1];
            amplitude[j]=sqrt(real*real+imag*imag);
        }
        fftw_destroy_plan(p);
        fftw_free(out);
        return amplitude;
    }
    if(showAliasing&&discrete&& getAlias)
    {
        if(orgIn != NULL)
            free(orgIn);
        orgIn=(double*)malloc(sizeof(double)*(number));
        for(int j=0;j<number;j++)
            orgIn[j]=data[j];
        out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(number));
        p=fftw_plan_dft_r2c_1d(number,orgIn,out,FFTW_ESTIMATE);
        fftw_execute(p);
        free(orgIn);
        double real,imag;
        amplitude2=(double*)malloc(sizeof(double)*(currentNumItems/2));
        for(int j=0;j<currentNumItems/2;j++)
        {
            real=out[j][0];
            imag=out[j][1];
            amplitude2[j]=sqrt(real*real+imag*imag);
        }
        fftw_destroy_plan(p);
        fftw_free(out);
        return amplitude2;
    }
    if((((downSample==0)||(downSample==1))&&(L==1)&&!bits)&&!replace)//org FFT
    {
        out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(num));
        p=fftw_plan_dft_r2c_1d(num,in,out,FFTW_ESTIMATE);
        fftw_execute(p);
    }
    else
    {
        out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(currentNumItems));
        in1=(double*)malloc(sizeof(double)*(currentNumItems));
        for(int i=0;i<num;i++)
        {
            in1[i]=0;
            if(i<currentNumItems)
            in1[i]=plotData[i];
        }
        p=fftw_plan_dft_r2c_1d((currentNumItems),in1,out,FFTW_ESTIMATE);
        fftw_execute(p);
        free(in1);
    }
    double real,imag;
    amplitude=(double*)malloc(sizeof(double)*(currentNumItems/2));
    for(int j=0;j<currentNumItems/2;j++)
    {
        real=out[j][0];
        imag=out[j][1];
        amplitude[j]=sqrt(real*real+imag*imag);
    }
    free(orgIn);
    fftw_destroy_plan(p);
    fftw_free(out);
    return amplitude;
}

void GLOSpectrum::plotXAxis()
{
    xLabels = xlabel->getLabels(true);
    int n = number;
    int numOftick = xlabel->size;
    xCor = xlabel->getCord(xMax,  n,xScaleW1,xShiftW1, hshift, width,hIn);
    glBegin(GL_LINES);
    for(int i=0;i<numOftick;i++)
    {
      if((xCor[i])>xShiftW1&&(xCor[i])<width)
      {
            glVertex2f(xCor[i],height/11);
            glVertex2f(xCor[i],height/15);
      }
    }
    glEnd();
    QFont f;
    float c = height / 30.0;
    if(discrete)
    {
        for(int i=0;i<numOftick;i++)
            {
                f.setPixelSize(10);
                if((xCor[i])-10>xShiftW1&&(xCor[i]-10)<width)
                {
                    int ii;
                    if( i != numOftick-1 )
                        ii=i+1;
                    else
                        ii=i-1;
                    if(fabs(xLabels[i]-xLabels[ii])==-1)
                        renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i]-1,'f',2),f );
                    else if(fabs(xLabels[i]-xLabels[ii])>=0.1)
                    {
                        renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i]-1,'f',2),f );
                    }else if((fabs(xLabels[i]-xLabels[ii])<0.1)&&(fabs(xLabels[i]-xLabels[ii])>=0.01))
                    {
                        renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i]-1,'f',3),f );
                    }else if((fabs(xLabels[i]-xLabels[ii])<0.01)&&(fabs(xLabels[i]-xLabels[ii])>=0.001))
                    {
                        f.setPixelSize(9);
                        renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i]-1,'f',3),f );
                        f.setPixelSize(10);
                    }
                    else
                        renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i]-1,'f',3),f );
                }
            }
    }
    else
    {
        for(int i=0;i<numOftick;i++)
            {
                f.setPixelSize(10);
                if((xCor[i])-10>xShiftW1&&(xCor[i]-10)<width)
                {
                    int ii;
                    if( i != numOftick-1 )
                        ii=i+1;
                    else
                        ii=i-1;
                    if(fabs(xLabels[i]-xLabels[ii])==-1)
                        renderText ( xCor[i]-10, c, 0, QString::number( (xLabels[i]-1)*sr/2*L/downSample,'f',2),f );
                    else if(fabs(xLabels[i]-xLabels[ii])>=0.1)
                    {
                        renderText ( xCor[i]-10, c, 0, QString::number( (xLabels[i]-1)*sr/2*L/downSample,'f',2),f );
                    }
                    else if((fabs(xLabels[i]-xLabels[ii])<0.1)&&(fabs(xLabels[i]-xLabels[ii])>=0.01))
                    {
                        renderText ( xCor[i]-10, c, 0, QString::number( (xLabels[i]-1)*sr/2*L/downSample,'f',3),f );
                    }else if((fabs(xLabels[i]-xLabels[ii])<0.01)&&(fabs(xLabels[i]-xLabels[ii])>=0.001))
                    {
                        f.setPixelSize(9);
                        renderText ( xCor[i]-10, c, 0, QString::number( (xLabels[i]-1)*sr/2*L/downSample,'f',3),f );
                        f.setPixelSize(10);
                    }
                    else
                        renderText ( xCor[i]-10, c, 0, QString::number( (xLabels[i]-1)*sr/2*L/downSample,'f',3),f );
                }
            }
    }
}

void GLOSpectrum::plotAxis()
{
    QFont f;
    f.setPixelSize(10);
    float c = height / 30.0;
    if(!discrete)
    {
       float xCor1=currentNumItems/2/(xScaleW1)-hshift+xShiftW1-(currentNumItems/2)/(xScaleW1*hIn);
       float xCor2=currentNumItems/2/(xScaleW1)-hshift+(currentNumItems/2)/(xScaleW1*hIn)+xShiftW1;
       float xCor3=number/2/(xScaleW1)-hshift+xShiftW1;
       glBegin(GL_LINES);
       glVertex2f(xCor1,height/11);
       glVertex2f(xCor1,height/13);
       glEnd();
       glBegin(GL_LINES);
       glVertex2f(xCor2,height/11);
       glVertex2f(xCor2,height/13);
       glEnd();
       glBegin(GL_LINES);
       glVertex2f(xCor3,height/11);
       glVertex2f(xCor3,height/13);
       glEnd();
       QString wn=QString::number(sr*500,'f',0);
       renderText(xCor2-10,3,0,wn,f);
       renderText(xCor3-10,3,0,QString::number(0,'f',0),f);
       renderText(xCor1-10,3,0,"-"+wn,f);
    }
    if(discrete)
    {
        float xCor1=(number/2)/(xScaleW1)-hshift+xShiftW1-(currentNumItems/2)/(xScaleW1*hIn);
        float xCor2=number/2/(xScaleW1)-hshift+(currentNumItems/2)/(xScaleW1*hIn)+xShiftW1;
        float xCor3=number/2/(xScaleW1)-hshift+xShiftW1;
        if(numofP<=10)
        {
            glBegin(GL_LINES);
            glVertex2f(xCor1,height/11);
            glVertex2f(xCor1,height/13);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(xCor2,height/11);
            glVertex2f(xCor2,height/13);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(xCor3,height/11);
            glVertex2f(xCor3,height/13);
            glEnd();
            renderText(xCor1,4,0,QString::number(-1,'f',0),f);
            renderText(xCor2,4,0,QString::number(1,'f',0),f);
            renderText(xCor3,4,0,QString::number(0,'f',0),f);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2f(xCor3,height/11);
            glVertex2f(xCor3,height/13);
            glEnd();
            renderText(xCor3,4,0,QString::number(0,'f',0),f);
        }
        glBegin(GL_LINES);
        float xCor=xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
        int k=2;
        if(numofP>=20&&numofP<55)
            k=4;
        else if(numofP>=55&&numofP<100)
            k=8;
        else if(numofP>100)
            k=16;
        while(xCor>xShiftW1+20)
        {
            if(xCor>xShiftW1+20)
            {
                glVertex2f((currentNumItems/2-currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)),height/11);
                glVertex2f((currentNumItems/2-currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)),height/13);
            }
          xCor=(currentNumItems/2-currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
          if(number/currentNumItems>=11&&number/currentNumItems<20)
            k+=2;
             else if(number/currentNumItems<=13)
                k++;
          else if(numofP>=20&&numofP<55)
              k+=4;
          else if(numofP>=55&&numofP<100)
              k+=8;
          else k+=16;
        }
       xCor=xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
        k=2;
        if(numofP>=20&&numofP<55)
            k=4;
        else if(numofP>=55&&numofP<100)
            k=8;
        if(numofP>100)
            k=16;
        while(xCor<width)
        {
            glVertex2f((currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)),height/11);
            glVertex2f((currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)),height/13);
            xCor=(currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
            if(number/currentNumItems>=11&&number/currentNumItems<20)
                k+=2;
            else if(number/currentNumItems<=13)
                k++;
            else if(numofP>=20&&numofP<55)
                k+=4;
            else if(numofP>=55&&numofP<100)
                k+=8;
            else k+=16;
        }
         glEnd();
         xCor=xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
          k=2;
         if(numofP>=20&&numofP<55)
             k=4;
         else if(numofP>=55&&numofP<100)
             k=8;
         else if(numofP>100)
             k=16;
         while(xCor>xShiftW1+20)
         {
             if(xCor>xShiftW1+40)
             {
                if(numofP>13&&numofP<55)
                    renderText((currentNumItems/2-(currentNumItems/2)*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-7,c,0,QString::number(-k,'f',0),f);
                 else if(number/currentNumItems<=13)
                     renderText((currentNumItems/2-(currentNumItems/2)*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-5,c,0,QString::number(-k,'f',0),f);
                else
                    renderText((currentNumItems/2-currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))-10,c,0,QString::number(-k,'f',0),f);
             }
                 xCor=(currentNumItems/2-(currentNumItems/2)*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));

                 if(number/currentNumItems>=11&&number/currentNumItems<20)
                     k+=2;
                 else if(number/currentNumItems<=13)
                     k++;
                 else if(numofP>=20&&numofP<55)
                     k+=4;
                 else if(numofP>=55&&numofP<100)
                     k+=8;
                 else k+=16;
         }
         xCor=xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
         k=2;
         if(numofP>=20&&numofP<55)
             k=4;
         else if(numofP>=55&&numofP<100)
             k=8;
         else if(numofP>100)
             k=16;
     while(xCor<width)
     {
         if(numofP>13&&numofP<55)
            renderText((currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))+5,c,0,QString::number(k,'f',0),f);
         else if(number/currentNumItems<=13)
            renderText((currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)),c,0,QString::number(k,'f',0),f);
         else
            renderText((currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)),c,0,QString::number(k,'f',0),f);
         xCor=(currentNumItems/2+currentNumItems/2*k)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn));
         if(number/currentNumItems>=11&&number/currentNumItems<20)
            k+=2;
         else if(number/currentNumItems<=13)
            k++;
         else if(numofP>=20&&numofP<55)
            k+=4;
         else if(numofP>=55&&numofP<100)
            k+=8;
         else k+=16;
     }
   }
}

int GLOSpectrum::getIndex(float value)
{
    int index;
    if(value<0)
        index=number/2*(1+value);
    else
        index=number/2*(value);
    return index;
}

int GLOSpectrum::normalizeMax(float max)
{
    float cmax=max;
    if(fabs(max)<=10&&fabs(max)>=1)
    {
        cmax=max;
        return cmax;
    }
    else if(fabs(max)>10)
    {
        int n=0;
        while(fabs(cmax)/10>10)
        {
            n++;
            cmax/=10;
        }
        int plot=max/pow(10,n+1)+2;
        return plot;
    }
    else
    {
        float cmax1=max;
        int n=0;
        while(cmax1<1)
        {
            cmax1*=10;
            n++;
        }
        int plot=max*pow(10,n+1)+1;
                return plot;
    }
}

void GLOSpectrum::ygenerateAxis()
{
    if(discrete)
    {
        uint n = height/20;
        if (n < 1) { n = 1; }
        double S[]={1,2,5};
        uint i = 0;
        uint grid = 1;
        int current = ceil(_max);
        while(!((int)(n*grid)>(current)))
        {
          ++i;
          grid = pow(10,i/3)*S[i%3];
        }
        int yGrid = grid;
        int numOfTicks = current*vIn/yGrid;
        glBegin(GL_LINES);
        QFont f;
        f.setPixelSize(10);
            if(yl.size()!=0)
            yl.clear();
        if(ycord.size()!=0)
        ycord.clear();
        int step = current*vIn / numOfTicks;
        float step2 =(float)_max/numOfTicks;
        for(int i=0;i<numOfTicks;i++)
        {
            yl.append(i*step2);
        }

        for(int j = 0,i=0; j<=current*vIn;j+=yGrid,i++)
        {
            double yPos = (i)*step*yScales+yShiftW1-vshift;
            ycord.append(yPos);
            yl.append(j);
            glVertex2f(xShiftW1,yPos);
            glVertex2f(xShiftW1-width/70,yPos);

        }
        glEnd();
        for(int i =0;i<numOfTicks;i++)
        {
            if(yl.value(6) > 100)
                renderText (2 ,ycord[i] , 0, QString::number(yl[i],'f',0),f);
            else if(yl.value((6)> 10 && yl.value(6)<100))
                renderText (2 ,ycord[i] , 0, QString::number(yl[i],'f',1),f);
            else
                renderText (2 ,ycord[i] , 0, QString::number(yl[i],'f',2),f);
        }
    }
    else
    {
        uint n = height/20;
        if (n < 1) { n = 1; }
        double S[]={1,2,5};
        uint i = 0;
        uint grid = 1;
        int current = ceil(_max);
        while(!((int)(n*grid)>(current)))
        {
          ++i;
          grid = pow(10,i/3)*S[i%3];
        }
        int yGrid = grid;
        int numOfTicks = current*vIn/yGrid;
        glBegin(GL_LINES);
        QFont f;
        f.setPixelSize(10);
        if(yl.size()!=0)
        {
           yl.clear();
        }
        if(ycord.size()!=0)
        {
           ycord.clear();
        }
        int step = current*vIn / numOfTicks;
        float step2 =(float)_max/numOfTicks;
        for(int i=0;i<numOfTicks;i++)
        {
            yl.append(i*step2/sr);
        }
        for(int j = 0,i=0; j<=current*vIn;j+=yGrid,i++)
        {
            double yPos = (i)*step*yScales+yShiftW1-vshift;
            ycord.append(yPos);
            yl.append(j);
            glVertex2f(xShiftW1,yPos);
            glVertex2f(xShiftW1-width/70,yPos);
        }
        glEnd();
        for(int i =0;i<numOfTicks;i++)
        {
            if(yl.value(6) > 100)
                renderText (2 ,ycord[i] , 0, QString::number(yl[i],'f',0),f);
            else if(yl.value((6)> 10 && yl.value(6)<100))
                renderText (2 ,ycord[i] , 0, QString::number(yl[i],'f',1),f);
            else
                renderText (2 ,ycord[i] , 0, QString::number(yl[i],'f',2),f);
        }
    }
}

void GLOSpectrum::yPlotAxis(float yaxisFactor)
{
    int numOftick= normalizeMax(_max)/yaxisFactor;
    yLabels=(float*)malloc(sizeof(float)*numOftick);
    QFont f;
    f.setPixelSize(10);
    yCor=(float*)malloc(sizeof(float)*numOftick);
    for(int i=0;i<numOftick;i++)
    {
        yLabels[i]=0;
        yCor[i]=0;
    }
     glBegin(GL_LINES);
     for(int i=0;i<numOftick;i++)
     {
         if(discrete)
             yLabels[i]+=yaxisFactor*(i);
         else
             yLabels[i]+=yaxisFactor*(i);
             yCor[i]=yLabels[i]*yScales*vIn*100+yShiftW1-vshift;
             glVertex2f(xShiftW1,yCor[i]);
             glVertex2f(xShiftW1-width/70,yCor[i]);
      }
     glEnd();
     for(int i=0;i<numOftick;i++)
     {
         if(discrete)
            renderText (2 ,yCor[i] , 0, QString::number( yLabels[i],'f',2),f);
         else
            renderText (2 ,yCor[i] , 0, QString::number( yLabels[i]*sr,'f',0),f);
     }
}

void GLOSpectrum::setRipple(QString ripple)
{
    this->ripple=ripple.toFloat();
    if(this->ripple <= 0)
    {
        if(filterData!=NULL)
            free(filterData);
        filterData=(float*)malloc(number*sizeof(float));
        emit RippleNotValid(true);
        return;
    }
    else emit RippleNotValid(false);
}

void GLOSpectrum::setCutofFreq(QString s)
{
    int cut=s.toFloat();
    NyqFreq=cut/sr/1000;
}

float inline GLOSpectrum::getValue(float xCor)
{
    float xValue=(xCor-(xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))))*(xScaleW1*hIn)-number/2;
    return xValue;
}

float inline GLOSpectrum::getCor(float xValue)
{
    float x=(xValue)/(xScaleW1*hIn)+xShiftW1-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn));
    return x;
}

void GLOSpectrum::computeCurrent()
{
    if(hIn<=1)
    {
        int index1 = (-((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn))+hshift)*xScaleW1*hIn;
        currentMin = (float)index1 / currentNumItems ;
        int index2 = (width - ((width-xShiftW1)/2-(currentNumItems/2)/(xScaleW1*hIn)) - xShiftW1 + hshift)* (xScaleW1*hIn);
        currentMax = (float)index2 / currentNumItems  ;
        xlabel->setCurrent(currentMin, currentMax);
    }
}

void GLOSpectrum::hIncrease()
{
    if(hIn>xFactor2)
    {
        hIn=hIn-xFactor2;
        setHshift(orgShift);
        timeOfScale++;
    }
    else
    {
        xFactor2/=2;
        hIncrease();
    }
}

void GLOSpectrum::resetSpec(float *data, int number, int sr)
{
    this->data = data;
    this->number = number;
    this->sr = (float)sr/1000;
    _max=0;
    showRange=false;
    factor=1;
    NyqFreq=1;
    order=2;
    showFilter=false;
    ripple=0.5;
    conFirst=false;
    disFirst=false;
    currentIndex=0;
    yTimeOfScale=0;
    currentNumItems = number;
    replace=false;
    timeOfScale = 0;
    showRange = false;
    showAliasing = false;
    showOnePeriod = false;
    getAlias = false;
    bits = 0;
}

void GLOSpectrum::show1Period(bool i)
{
    showOnePeriod = i;
    updateGL();
}

void GLOSpectrum::setShowAliasing(bool i)
{
    this->showAliasing = i;
    updateGL();
}

void GLOSpectrum::setBanned(bool i)
{
    banned = i;
}

bool GLOSpectrum::get1Period()
{
    return showOnePeriod;
}
void GLOSpectrum::setFactor(QString s)
{
    factor=s.toFloat()/(float)sr/1000;
}

inline int GLOSpectrum::getIndex1(float value)
{
    int index=(value -xShiftW1 +(currentNumItems/2)/xScaleW1*hIn-(width-xShiftW1)/2 +hshift)*(xScaleW1*hIn);
    if(index<0)
        index+=currentNumItems/2;
    return index;
}

GLOSpectrum* GLOSpectrum::getWidget()
{
    return this;
}
