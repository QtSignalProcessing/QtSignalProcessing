#include "audiohandle.h"
#include<iostream>
#include"fftw3.h"
#include <cmath>


using namespace std;
AudioHandle::AudioHandle(const QString& s)
{
    info.format = 0;
    sf = sf_open(s.toStdString().c_str(),SFM_READ,&info);
    if (sf == NULL)
    {
        cout<<"Failed to open the file. "<<sf_strerror(sf)<<" "<<sf_error(sf)<<endl;
        info.channels = 1;
        info.frames = 20000;
        info.samplerate = 11025;
        info.format = 65541;
        info.sections = 1;
        info.seekable =1;
    }
}

AudioHandle::~AudioHandle(){}

int AudioHandle::getFrameNum()
{
    return info.frames;
}

int AudioHandle::getSamplerate()
{
    return info.samplerate;
}

int AudioHandle::getChannel()
{
    return info.channels;
}

QList<float> AudioHandle::getData(int num_items,int& num)
{
    QList<float> data(num_items);
    if(sf == nullptr)
    {
        num = 20000;
        return {};
    }
    num=sf_readf_float(sf,data.data(),num_items);
    return data;
}

void AudioHandle::writeToWave(float *data, char *filename,float sr,int size)
{
    SNDFILE   *outfile ;
    SF_INFO      sfinfo ;
    sfinfo.channels=1;
    sfinfo.samplerate=sr;
    sfinfo.format=info.format;
    if (! (outfile = sf_open (filename, SFM_WRITE,&sfinfo)))
    {   printf ("Not able to open output file %s.\n", filename) ;
        sf_perror (NULL) ;
        return   ;
        } ;
    sf_write_float (outfile, data, size) ;
    sf_close(outfile);
}

QList<float> AudioHandle::triangularMagnitude(QList<float> data,int size)
{
    fftw_complex *out;
    fftw_plan p;
    double *orgIn=(double*)malloc(sizeof(double)*(size));
    for(int j=0;j<size;j++)
        orgIn[j]=data[j];

    out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(size));
    p=fftw_plan_dft_r2c_1d(size,orgIn,out,FFTW_ESTIMATE);
    fftw_execute(p);
    double real,imag;
    double phase;
    double step = 1.0 / size/2;
    double magnitude = 0;
    for(int j=0;j<size/2;j++)
    {
        real=out[j][0];
        imag=out[j][1];
        magnitude = (float)j * step;

        phase = atan2(imag,real);
        real = magnitude * cos(phase);
        imag = magnitude * sin(phase);
        out[j][0] = real;
        out[j][1] = imag;
        out[size-j][0] =real;
        out[size-j][1] = imag;
    }

    if(_triangular_wave.size() != 0)
        _triangular_wave.clear();
    _triangular_wave.resize(size);

    fftw_complex * in1 = (fftw_complex*)malloc(sizeof(fftw_complex)*size);

    p = fftw_plan_dft_1d(size,out,in1,FFTW_BACKWARD,FFTW_ESTIMATE);
    fftw_execute(p);
    free(orgIn);
    fftw_destroy_plan(p);
    fftw_free(out);
    for(int i = 0;i<size/2;i++)
    {
        _triangular_wave[i] = in1[size/2-i][0];
        _triangular_wave[size -i-1]=in1[size/2-i][0];
    }
    free(in1);
    return _triangular_wave;
}
