#include "utilities.h"
#include <samplerate.h>
#include "fftw3.h"
#include<stdlib.h>
#include<math.h>
#include<iostream>
#include"iir.h"
#include"chebyshevfilter.h"
#include <stdio.h>
#include <QDebug>

using namespace std;
Utilities::Utilities( float* data,int number):bits(0),L(1),down(1),data(data),qData(NULL),
    sampleData(NULL),filterData(NULL),SNR(0),number(number),currentNumItems(number),replace(false),filtered(false),_trueBits(0),_butter(true),_NyqFreq(0),_order(3),_ripple(0.5),_factor(1.0)
{
}

Utilities::~Utilities()
{
    if( qData != NULL )
        free(qData);
    if( sampleData != NULL )
        free(sampleData);
    if( filterData != NULL )
        free(filterData);
}

float* Utilities::getQuantize()
{
    if((bits==0&&(down==1)&&(L==1))&&(bits == _trueBits))
    {
        SNR=0;
        return data;
    }
    if(((bits == 0) && (L>2)))
    {
        SNR= 0 ;
        return getSampleData(true);
    }
    currentNumItems=number*(float)L/down;
    if(qData!=NULL)
        free(qData);
    qData=(float*)malloc(sizeof(float)*(currentNumItems));
    float max = 1;
    if((down==1)&&(L==1))
    {
        double num=pow(2,bits);
        float step=2*max/(float)num;
        float* level = (float*)malloc(sizeof(float)*num);
        for(int i=0;i<num;i++)
          {
              level[i]=-max+step*(float)i;
          }
        float index;
        for(int i=0;i<currentNumItems;i++)
        {
            index=(data[i]-level[0])/step;
            index = qRound(index);
            if(index>(num-1))
                index=num-1;
            if(index<0)
                index=0;
            qData[i]=level[0]+(float)index*step;
        }
        free(level);
        return qData;
    }
    int num=pow(2,bits);
    float step= 2 * max / (float)num;

    float level[num];
    for(int i=0;i<num;i++)
      {
          level[i]=  -max + step * (float)i;
      }
   float index;
   float *computeData;
    if(down>=2)
   {
       computeData=sampleData;
   }
   else
   {
       computeData=filterData;
   }
    int idx;
    for(int i=0;i<currentNumItems;i++)
    {
        index=(computeData[i]-level[0])/step;
        idx = qRound(index);
        if(idx>(num-1))
            idx=num-1;
        if(idx<0)
            idx=0;
        qData[i]=level[idx];
    }
    return qData;
}

QVector<float> Utilities::getOrgQuan()
{
    QVector<float> qdata;
    float max = 1;
    double num=pow(2,bits);
    float step=2*max/(float)num;
    float* level = (float*)malloc(sizeof(float)*num);
    for(int i=0;i<num;i++)
    {
          level[i]=-max+step*(float)i;
    }
    float index;
    for(int i=0;i<number;i++)
    {
        index=(data[i]-level[0])/step;
        index = qRound(index);
        if(index>(num-1))
            index=num-1;
        if(index<0)
            index=0;
        qdata.push_back(level[0]+(float)index*step);
    }
    free(level);
    return qdata;
}

int Utilities::computeTrueBits()
{
    int i;
    for( i =1;i<=16;i++)
    {
        setBit(i);
        getQuantize();
        double error = computeError();
        if(error < 10e-6)
        {
            _trueBits = i;
            return i;
        }
    }
    _trueBits = i-1;
    bits = i-1;
    return i-1;
}

void Utilities::setBit(int bits)
{
    this->bits=bits;
}

void Utilities::setSampleFactor(int L, int down,int currentNumItems)
{
    this->L=L;
    this->down=down;
    this->currentNumItems=currentNumItems;
}

double Utilities::computeError()
{
    if(this->bits==0)
        return -1;
    double *error=(double*)malloc(sizeof(double)*(currentNumItems));
    float *computeData;
    if(down<2)
        computeData=data;
    else
        computeData=sampleData;
    for(int i=0;i<currentNumItems;i++)
    {
        error[i]=(qData[i]-computeData[i]);
    }
    double varErr = 0;
    for(int i=0;i<currentNumItems;i++)
    {
        varErr += pow( ( error[i] ), 2 );
    }
    varErr /= currentNumItems;
    varErr = sqrt( varErr );
    free(error);
    return varErr;
}

double Utilities::getSNR(double varErr)
{
    if(this->bits==0)
    {
        SNR=0;
        return SNR;
    }
    float *computeData;
    if(down<2)
        computeData=data;
    else
        computeData=sampleData;
    double varOrg = 0;

    for(int i=0;i<currentNumItems;i++)
    {
        varOrg+=pow((computeData[i]),2);
    }
    varOrg /= currentNumItems;
    varOrg = sqrt( varOrg );
    SNR=20 * log10f( varOrg / varErr );
    return SNR;
}

void Utilities::setReplace(bool replace)
{
    this->replace=replace;
}

float Utilities::getMax1(float *a,int num)
{
    float max=a[0];
    for(int i=1;i<num;i++)
        if(max<a[i])
            max=a[i];
    return max;
}

float* Utilities::getSampleData(bool org)
{
    if(sampleData!=NULL)
        free(sampleData);
    sampleData=(float*)malloc(sizeof(float)*currentNumItems);
    SRC_STATE	*src_state ;
    SRC_DATA	src_data ;
    int	error ;
    /* Initialize the sample rate converter. */
    if ((src_state = src_new (0, 1, &error)) == NULL)
    {
        printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
        exit (1) ;
    } ;
    src_data.input_frames =number;
    if(!org)
    {
        src_data.data_in=filterData;
    }
    else
        src_data.data_in=data;
    src_data.data_out=sampleData;
    src_data.src_ratio=(double)L/down;
    src_data.output_frames=currentNumItems;
    src_simple (&src_data,  4, 1) ;
   // if(filtered)
      //  return sampleData;
   if(bits != _trueBits)
    {
       qDebug()<<"I am here";
        return getQuantize();
    }
    return sampleData;
}

void Utilities::setFilterData(float *filterData)
{
    this->filterData=filterData;
}

void Utilities::updateUtilites(float *data, int number)
{
    this->data = data;
    this->number = number;
    L = 1;
    down = 1;
    currentNumItems= number;
    replace = false;
    filtered =false;
}

void Utilities::setOnlyFiltered(bool filtered)
{
    this->filtered = filtered;
}

const QVector<float> Utilities::getAmplitude(const QVector<float>& in)
{
    fftw_complex *out;
    fftw_plan p;
    double *orgIn=(double*)malloc(sizeof(double)*(in.size()));
    for(int j=0;j<in.size();j++)
    {
        orgIn[j]=in[j];
    }

    out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(in.size()));
    p=fftw_plan_dft_r2c_1d(in.size(),orgIn,out,FFTW_ESTIMATE);
    fftw_execute(p);
    double real,imag;
    QVector<float> amplitude;
    for(int j=in.size()/2;j >= 0;j--)
    {
        real=out[j][0];
        imag=out[j][1];
        amplitude.push_back(sqrt(real*real+imag*imag));
    }
    for(int j= 0 ;j < in.size()/2- 1;j++)
    {
        real=out[j][0];
        imag=out[j][1];
        amplitude.push_back(sqrt(real*real+imag*imag));
    }
    free(orgIn);
    fftw_destroy_plan(p);
    fftw_free(out);
    return amplitude;
}
const QVector<float> Utilities::getAmplitude()
{
    fftw_complex *out;
    fftw_plan p;
    double *orgIn=(double*)malloc(sizeof(double)*(number));
    for(int j=0;j<number;j++)
    {
        orgIn[j]=data[j];
    }
    out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(number));
    p=fftw_plan_dft_r2c_1d(number,orgIn,out,FFTW_ESTIMATE);
    fftw_execute(p);
    double real,imag;
    QVector<float> amplitude;
    for(int j=number/2;j >= 0;j--)
    {
        real=out[j][0];
        imag=out[j][1];
        amplitude.push_back(sqrt(real*real+imag*imag));
    }
    for(int j= 0 ;j < number/2- 1;j++)
    {
        real=out[j][0];
        imag=out[j][1];
        amplitude.push_back(sqrt(real*real+imag*imag));
    }
    free(orgIn);
    fftw_destroy_plan(p);
    fftw_free(out);
    return amplitude;
}

QVector<float> Utilities::getFilterData()
{
    QVector<float> tmp;
   // double factor = (double)L/down/2;
    if(_ripple <= 0 || _order >15)
        return tmp;

    if(filterData!=NULL)
        free(filterData);
    filterData=(float*)malloc(number*sizeof(float));
    for(int i=0;i<number;i++)
        filterData[i]=0;
    double* b1=(double*)malloc((_order+1)*sizeof(double));
    double* a1=(double*)malloc((_order+1)*sizeof(double));
    if(_butter)
    {
        IIR* iir=new IIR(0,_order,_factor,0,0);
        qDebug()<<_factor;
        for(int i=0;i<=_order;i++)
        {
            b1[i]=iir->b[i];
            a1[i]=iir->a[i];
        }
        delete iir;
    }
    else
    {
        ChebyshevFilter* d=new ChebyshevFilter(1,_order,_ripple,1.0,1.0*(_factor/2),0);
        for(int i=0;i<=_order;i++)
        {
            b1[i]=d->pp[i];
            a1[i]=d->aa[i];
        }
        delete d;
    }
    int i,j;
    filterData[0]=b1[0]*data[0];
    for (i=1;i<_order+1;i++)
    {
        filterData[i]=0.0;
        for (j=0;j<i+1;j++)
                filterData[i]=filterData[i]+b1[j]*data[i-j];
        for (j=0;j<i;j++)
                filterData[i]=filterData[i]-a1[j+1]*filterData[i-j-1];
    }
    /* end of initial part */
    for (i=_order+1;i<number+1;i++)
    {
        filterData[i]=0.0;
        for (j=0;j<_order+1;j++)
        filterData[i]=filterData[i]+b1[j]*data[i-j];
        for (j=0;j<_order;j++)
        filterData[i]=filterData[i]-a1[j+1]*filterData[i-j-1];
    }
    free(a1);
    free(b1);
    if(fabs(getMax1(filterData,number))>3)
    {
        for(int i = 0;i < number; i++)
            filterData[i] = data[i];
    }

    for( int i = 0;i < number; i++ )
        tmp.push_back(filterData[i]);
    return tmp;
}

void Utilities::butterOrCheby(bool i)
{
    _butter = i;
}

void Utilities::setOrder(int order)
{
    _order = order;
}

void Utilities::setRipple(float ripple)
{
    _ripple = ripple;
}

void Utilities::setFactor(float factor)
{
    _factor = factor;
}

void Utilities::setNyqFreq(double s)
{
    _NyqFreq = s;
}

double Utilities::getNyq()
{
    return _NyqFreq;
}
