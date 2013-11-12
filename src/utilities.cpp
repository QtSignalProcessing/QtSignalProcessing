#include "utilities.h"
#include<stdlib.h>
#include<math.h>
#include<iostream>
using namespace std;
Utilities::Utilities(float* data,int number):bits(0),L(1),down(1),data(data),qData(NULL),
    sampleData(NULL),filterData(NULL),SNR(0),number(number),currentNumItems(number),replace(false),filtered(false),_trueBits(0)
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
    if(bits==0&&!(down>=2))
    {
        SNR=0;
        return data;
    }
    if((bits == 0) && (L>2))
    {
        SNR= 0 ;
        return sampleData;
    }
    currentNumItems=number*(float)L/down;
    if(qData!=NULL)
        free(qData);
    qData=(float*)malloc(sizeof(float)*(currentNumItems));
    float max = 1;
    if((down==1)&&(L==1))
    {
        int num=pow(2,bits);
        float step=2*max/(float)num;
        float level[num];
        for(int i=0;i<num;i++)
          {
              level[i]=-max+step*(float)i;
          }
        int index;
        for(int i=0;i<currentNumItems;i++)
        {
            index=(data[i]-level[0])/step;
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
   int index;
   float *computeData;
    if(down>=2)
   {
       computeData=sampleData;
   }
   else
   {
       computeData=filterData;
   }

    for(int i=0;i<currentNumItems;i++)
    {
        index=(computeData[i]-level[0])/step;
        if(index>(num-1))
            index=num-1;
        if(index<0)
            index=0;
        qData[i]=level[index];
    }
    return qData;
}

int Utilities::computeTrueBits()
{
    int i;
    for( i =1;i<=16;i++)
    {
        setBit(i);
        getQuantize();
        double error = computeError();
        if(error < 10e-5)
        {
            _trueBits = i;
            return i;
        }
    }
    return 0;
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
    sampleData=(float*)malloc(sizeof(float)*currentNumItems+1);
    SRC_STATE	*src_state ;
    SRC_DATA	src_data ;
    int	error ;
    /* Initialize the sample rate converter. */
    if ((src_state = src_new (0, 1, &error)) == NULL)
    {	printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
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
    if(filtered)
        return sampleData;
    if(bits != _trueBits)
    {
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
