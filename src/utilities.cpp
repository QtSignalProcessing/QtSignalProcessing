#include "utilities.h"
#include "iir.h"
#include "chebyshevfilter.h"

#include <samplerate.h>
#include <fftw3.h>

#include <cstdlib>
#include <cmath>
#include <iostream>
//#include <stdio.h>
#include <QDebug>

using namespace std;
Utilities::Utilities(QList<float>& data,int number):bits(0),L(1),down(1), data(data),
    SNR(0),number(number),currentNumItems(number),replace(false),filtered(false),_trueBits(0),_butter(true),_NyqFreq(0),_order(3),_ripple(0.5),_factor(1.0)
{
}

Utilities::~Utilities(){}

QList<float> Utilities::getQuantize()
{
    if((bits == 0 && (down == 1) && (L == 1)) && (bits == _trueBits))
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

    if(qData.size() != 0)
        qData.clear();

    qData.resize(currentNumItems);
    float max = 1;
    if((down == 1) && (L == 1))
    {
        double num = pow(2,bits);
        float step=2*max/(float)num;
        QVector<float> level;
        for(int i=0;i<num;i++)
        {
            level.push_back(-max+step*(float)i);
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
    QList<float> computeData;
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

QList<float> Utilities::getOrgQuan()
{
    QList<float> qdata;
    float max = 1;
    int num = pow(2,bits);
    float step = 2 * max / (float)num;
    float level[num];
    for(int i = 0; i < num; i++)
    {
        level[i]=-max+step*(float)i;
    }
    float index;
    for(int i = 0; i < number; i++)
    {
        index = (data[i]-level[0])/step;
        index = qRound(index);
        if(index>(num-1))
            index=num-1;
        if(index<0)
            index=0;
        qdata.push_back(level[0]+(float)index*step);
    }
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
    QList<double> error(currentNumItems);
    QList<float> computeData;
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

    return varErr;
}

double Utilities::getSNR(double varErr)
{
    if(this->bits==0)
    {
        SNR=0;
        return SNR;
    }

    QList<float> computeData;
    if(down<2)
        computeData = data;
    else
        computeData = sampleData;
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

float Utilities::getMax1(const QList<float>& a,int num)
{
    float max=a[0];
    for(int i=1;i<num;i++)
        if(max<a[i])
            max=a[i];
    return max;
}

QList<float> Utilities::getSampleData(bool org)
{
    if(sampleData.size() != 0)
        sampleData.clear();
    sampleData.resize(currentNumItems);
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
        src_data.data_in = filterData.data();
    }
    else
        src_data.data_in = data.data();
    src_data.data_out = sampleData.data();
    src_data.src_ratio= (double)L / down;
    src_data.output_frames = currentNumItems;
    src_simple (&src_data,  4, 1) ;

   if(bits != _trueBits)
    {
        return getQuantize();
    }
    return sampleData;
}

void Utilities::setFilterData(QList<float> filterData)
{
    this->filterData = filterData;
}

void Utilities::updateUtilites(QList<float> data, int number)
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

const QList<float> Utilities::getAmplitude(const QList<float>& in)
{
    fftw_complex *out;
    fftw_plan p;
    double* orgIn = new double[sizeof(double)*(number)];
    for(int j=0;j<in.size();j++)
    {
        orgIn[j]=in[j];
    }

    out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(in.size()));
    p=fftw_plan_dft_r2c_1d(in.size(),orgIn,out,FFTW_ESTIMATE);
    fftw_execute(p);
    double real,imag;
    QList<float> amplitude;
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
    delete[] orgIn;
    fftw_destroy_plan(p);
    fftw_free(out);
    return amplitude;
}
const QList<float> Utilities::getAmplitude()
{
    fftw_complex *out;
    fftw_plan p;
    double* orgIn = new double[sizeof(double)*(number)];
    for(int j=0;j<number;j++)
    {
        orgIn[j]=data[j];
    }
    out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(number));
    p=fftw_plan_dft_r2c_1d(number,orgIn,out,FFTW_ESTIMATE);
    fftw_execute(p);
    double real,imag;
    QList<float> amplitude;
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
    delete[] orgIn;
    fftw_destroy_plan(p);
    fftw_free(out);
    return amplitude;
}

QList<float> Utilities::getFilterData()
{
    QList<float> tmp;
    if(_ripple <= 0 || _order >15)
        return tmp;

    if(filterData.size() != 0)
        filterData.clear();

    filterData.resize(number);
    for(int i=0;i<number;i++)
        filterData[i]=0; 
    QList<double> b1, a1;
    if(_butter)
    {
        IIR* iir=new IIR(0,_order,_factor,0,0);

        for(int i=0;i<=_order;i++)
        {
            b1.push_back(iir->b[i]);
            a1.push_back(iir->a[i]);
        }
        delete iir;
    }
    else
    {
        ChebyshevFilter* d=new ChebyshevFilter(1,_order,_ripple,1.0,1.0*(_factor/2),0);
        for(int i = 0; i <= _order; i++)
        {
         //   qDebug() << i << " sdkfjhsdkfjskdljfklsd";
            //b1[i]=d->pp[i];
            b1.push_back(d->pp[i]);
            //a1[i]=d->aa[i];
            a1.push_back(d->aa[i]);
        }
        delete d;
    }
    int i,j;
    filterData[0] = b1[0] * data[0];
    for(i = 1; i < _order + 1; i++)
    {
        filterData[i] = 0.0;
        for (j = 0; j <i+1; j++)
                filterData[i]=filterData[i]+b1[j]*data[i-j];
        for (j=0;j<i;j++)
                filterData[i]=filterData[i]-a1[j+1]*filterData[i-j-1];
    }
    /* end of initial part */
    for (i = _order + 1; i < number; i++)
    {
        filterData[i]=0.0;
        for (j=0;j<_order+1;j++)
        filterData[i]=filterData[i]+b1[j]*data[i-j];
        for (j=0;j<_order;j++)
        filterData[i]=filterData[i]-a1[j+1]*filterData[i-j-1];
    }

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


int gcd(int v1,int v2)
{
    while(v2)
    {
        int temp=v2;
        v2=v1%v2;
        v1=temp;
    }
    return v1;
}
