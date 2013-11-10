#include "glsw.h"
#include <QtGui>
#include <QtOpenGL>
#include <QDebug>
#include <math.h>

#include<iostream>

using namespace std;
void DrawCircle(float cx, float cy, float r, int num_segments) ;


GLSW::GLSW(float *buf,int num,float time,bool sample,QWidget *parent)
    : GLBase(buf,num,time,sample,sr,parent),SNR(0),r(4),_stickOn(false)

{}

GLSW::~GLSW()
{}

void GLSW::paintGL()
{
    xScaleW1=number/(width-xShiftW1);
    yScaleW1=(float)(height-height/9)/max/2;
    yShiftW1=(height+height/10)/2;
    ylabel->autoScale();
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(5);
    if(!sampleOrNot)
         glColor3f(0,0,0);
    else
         glColor3f(0,0,1);
    glLineWidth(1.0f);
    float *data1;
    xlabel->autoScale();
    float ptDist = (float)number/2/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
    int previousIndex = 0;
    float x=number/2;
    if(sampleOrNot)
    {
        int num_segments=12;
        r=height/70;
        if(((downSample==1)&&(L==1)&&(!bits))&&!replace)
        {
            data1=data;
        }
        else{
               data1=plotData;
            }
       for (int i=currentNumItems/2; i>=0; x-=(float)downSample/L,i--)//down samping by f(x)=g(Mk) (here a is M)
       {
          if((x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1>=xShiftW1)&&(data1[i]*(yScaleW1*vIn)+yShiftW1+vshift>height/9))
          {
             float xCord=x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1;
             if(fabs(getDist(ptDist,data1[ previousIndex ]*(yScaleW1*vIn)+yShiftW1+vshift,xCord,data1[i]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
             {
               DrawCircle(xCord, data1[i]*(yScaleW1*vIn)+yShiftW1+vshift,r,num_segments);
               ptDist = xCord;
               previousIndex = i;
             }
         }
       }
       x=number/2;
       ptDist = (float)number/2/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
       previousIndex = 0;
       for (int i=currentNumItems/2; i<currentNumItems; x+=(float)downSample/L,i++)//down samping by f(x)=g(Mk) (here a is M)
          {
              float xCord=x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1;
              if((x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1>=xShiftW1)&&(data1[i]*(yScaleW1*vIn)+yShiftW1+vshift>height/9))
                 if(fabs(getDist(ptDist,data1[ previousIndex ]*(yScaleW1*vIn)+yShiftW1+vshift,xCord,data1[i]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                  {
                    DrawCircle(xCord, data1[i]*(yScaleW1*vIn)+yShiftW1+vshift,r,num_segments);
                    ptDist = xCord;
                    previousIndex = i;
                  }
          }
       if(_stickOn)
       {
        glBegin(GL_LINES);
          x=number/2;
          ptDist = (float)number/2/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
          previousIndex = 0;
          for (int i=currentNumItems/2; i>=0; x-=(float)downSample/L,i--)//down samping by f(x)=g(Mk) (here a is M)
          {
              float xCord=x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1;
              if((x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1>=xShiftW1)&&((data1[i]*(yScaleW1*vIn)+yShiftW1+vshift>height/9)))
              {
                  if(fabs(getDist(ptDist,data1[previousIndex] *(yScaleW1*vIn)+yShiftW1+vshift,xCord,data1[i]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                   {
                      glVertex2f(xCord,yShiftW1+vshift);
                      glVertex2f(xCord, data1[i]*(yScaleW1*vIn)-r+yShiftW1+vshift);
                      ptDist = xCord;
                      previousIndex = i;
                   }
              }
              if((x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1>=xShiftW1)&&(data1[i]*(yScaleW1*vIn)+yShiftW1+vshift<height/9))
              {
                   glVertex2f(xCord,yShiftW1+vshift);
                   glVertex2f(xCord, height/9);
              }
           }
            x=number/2;
            ptDist = (float)number/2/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
           previousIndex = 0;
            for (int i=currentNumItems/2; i < currentNumItems ; x+=(float)downSample/L,i++)//down samping by f(x)=g(Mk) (here a is M)
              {
                 float xCord=x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1;
                 if((x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1>=xShiftW1)&&((data1[i]*(yScaleW1*vIn)+yShiftW1+vshift>height/9)))
                 {
                     if(fabs(getDist(ptDist,data1[ previousIndex ]*(yScaleW1*vIn)+yShiftW1+vshift,xCord,data1[i]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                     {
                     glVertex2f(xCord,yShiftW1+vshift);
                       glVertex2f(xCord, data1[i]*(yScaleW1*vIn)-r+yShiftW1+vshift);
                       ptDist = xCord;
                       previousIndex = i;
                     }
                 }
                 if((x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1>=xShiftW1)&&(data1[i]*(yScaleW1*vIn)+yShiftW1+vshift<height/9))
                 {
                       glVertex2f(xCord,yShiftW1+vshift);
                       glVertex2f(xCord, height/9);
                 }
              }
                glEnd();
       }

                glBegin(GL_LINE_STRIP);
                 for (int x = number/2; x >=0; x--)//down samping by f(x)=g(Mk) (here a is M)
                 {
                    float xCord=x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1;
                     glVertex2f(xCord,yShiftW1+vshift);
                 }
                 for (int x = number/2; x <number; x++)//down samping by f(x)=g(Mk) (here a is M)
                 {
                     float xCord=x/(xScaleW1*hIn)-hshift+(width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)+xShiftW1;
                     glVertex2f(xCord,yShiftW1+vshift);
                 }
                 glEnd();
    }
    else
    {
        data1=data;
        glBegin(GL_LINE_STRIP);
          ptDist = (float)number/2/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
           for(int x=number/2;x>=0;x--){
               float xCor= (float)x/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
               if((xCor>=xShiftW1)&&(data[x]*(yScaleW1*vIn)+yShiftW1+vshift>height/11))
                {

                   if(fabs(getDist(ptDist,data[ getIndex(ptDist) ]*(yScaleW1*vIn)+yShiftW1+vshift,xCor,data[x]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                    {
                        if(getIndex(ptDist)<0)
                             cout<<getIndex(ptDist)<<endl;
                        glVertex2f(xCor, data[x]*(yScaleW1*vIn)+yShiftW1+vshift);
                        ptDist = xCor;
                    }
                }
               if((xCor>=xShiftW1)&&(data[x]*(yScaleW1*vIn)+yShiftW1+vshift<=height/11))
                   if(fabs(getDist(ptDist,data[getIndex(ptDist)]*(yScaleW1*vIn)+yShiftW1+vshift,xCor,data[x]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                    {
                        glVertex2f(xCor, height/11);
                       ptDist = xCor;
                    }

           }
           glEnd();
           ptDist = (float)number/2/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
           glBegin(GL_LINE_STRIP);
           for(int x=number/2-1;x<number;x++)
           {
               float xCor= (float)x/(xScaleW1*hIn)-hshift+((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+xShiftW1;
               if((xCor>=xShiftW1)&&(data[x]*(yScaleW1*vIn)+yShiftW1+vshift>height/9))
               {
                   if(fabs(getDist(ptDist,data[getIndex(ptDist)]*(yScaleW1*vIn)+yShiftW1+vshift,xCor,data[x]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                   {
                        glVertex2f(xCor, data[x]*(yScaleW1*vIn)+yShiftW1+vshift);
                        ptDist = xCor;
                   }

               }
               if((xCor>=xShiftW1)&&(data[x]*(yScaleW1*vIn)+yShiftW1+vshift<=height/11))
               {
                   if(fabs(getDist(ptDist,data[getIndex(ptDist)]*(yScaleW1*vIn)+yShiftW1+vshift,xCor,data[x]*(yScaleW1*vIn)+yShiftW1+vshift)) >= 1)
                   {
                       glVertex2f(xCor, height/11);

                   }
               }
          }
       glEnd();
    }
    glBegin(GL_LINES);
    //x-axis
    glVertex2f(0, height/11);
    glVertex2f(width, height/11 );
    //y-axis
    glVertex2f(xShiftW1, 0);
    glVertex2f(xShiftW1, height);
    glEnd();
    plotAxis();
    ygenerateAxis();
    glPopMatrix();
    glPopAttrib();
}


void DrawCircle(float cx, float cy, float r, int num_segments)
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

void GLSW::setSampleFactors(float SNR, int bits)
{
     this->SNR=SNR;
     this->bits=bits;
}
void GLSW::setSNR(double SNR)
{

    emit newSNR(SNR);
}
void GLSW::ygenerateAxis()
{
    int numOftick=ylabel->size;
    yLabels=ylabel->getLabels();
    yCor=ylabel->getYcord(yScaleW1,vIn,yShiftW1,vshift);
    glBegin(GL_LINES);
    for(int i=0;i<numOftick;i++)
    {
        if(yCor[i]>xShiftW1&&yCor[i]<height)
        {
          glVertex2f(xShiftW1,yCor[i]);
          glVertex2f(xShiftW1-width/70,yCor[i]);
        }
     }
    glEnd();
    QFont f;
    f.setPixelSize(10);
    for(int i=0;i<numOftick;i++)
    {
        if(yCor[i]>xShiftW1&&yCor[i]<height)
            renderText (2 ,yCor[i] , 0, QString::number( yLabels[i],'f',2),f);
    }
}

void GLSW::plotAxis()
{
    xLabels = xlabel->getLabels();
    xCor = xlabel->getCord(time,  number,xScaleW1,xShiftW1, hshift, width,hIn);
    int numOftick = xlabel->size;

    glBegin(GL_LINES);
    for(int i=0;i<numOftick;i++)
    {
      if((xCor[i])>xShiftW1&&(xCor[i])<width)
      {
        glVertex2f(xCor[i],height/11);
        glVertex2f(xCor[i],height/13);
      }
     }
    glEnd();
    float c = height / 30;
    for(int i=0;i<numOftick;i++)
    {
        QFont f;
        f.setPixelSize(10);
        if((xCor[i])-10>xShiftW1&&(xCor[i]-10)<width)
        {
            int ii;
            if( i != numOftick-1 )
                ii=i+1;
            else
                ii=i-1;
            if((xLabels[i]-xLabels[ii])==-1)
                renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i],'f',2),f );
            else if(fabs(xLabels[i]-xLabels[ii])>=0.1)
            {
                renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i],'f',2),f );
            }else if((fabs(xLabels[i]-xLabels[ii])<0.1)&&(fabs(xLabels[i]-xLabels[ii])>=0.01))
            {
                renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i],'f',2),f );
            }else if((fabs(xLabels[i]-xLabels[ii])<0.01)&&(fabs(xLabels[i]-xLabels[ii])>=0.001))
            {

                renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i],'f',4),f );
            }
         else if((fabs(xLabels[i]-xLabels[ii])<0.001)&&(fabs(xLabels[i]-xLabels[ii])>0.0001))
        {
                       renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i],'f',4),f );
        }
            else
                renderText ( xCor[i]-10, c, 0, QString::number( xLabels[i],'f',5),f );
        }
    }
}
void GLSW::computeCurrent()
{
    int index1 = (-((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn))+hshift)*xScaleW1*hIn;
    currentMin = (float)index1 / number * time;
    int index2 = ( width - xShiftW1 - ((width-xShiftW1)/2-(number/2)/(xScaleW1*hIn)) + hshift ) * xScaleW1*hIn;
    currentMax = (float)index2 / number * time;
    xlabel->setCurrent(currentMin, currentMax);

}

inline int GLSW::getIndex(float value)
{
  int index=(value -xShiftW1 +(currentNumItems/2)/xScaleW1*hIn-(width-xShiftW1)/2 +hshift)*(xScaleW1*hIn);
  if(index<0)
      index+=currentNumItems/2;
  return index;
}

GLSW* GLSW::getGlwidget()
{
    return this;
}

void GLSW::setStickOn(bool i)
{
    _stickOn = i;
    updateGL();
}
