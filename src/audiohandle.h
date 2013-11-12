#ifndef AUDIOHANDLE_H
#define AUDIOHANDLE_H

#include<sndfile.h>
#include<stdio.h>
#include<stdlib.h>
#include<QString>
#include<QDir>
class AudioHandle
{

public:
    SNDFILE *sf;
    SF_INFO info;
    AudioHandle(const char* s);
    ~AudioHandle();
    int getFrameNum();
    int getSamplerate();
    int getChannel();
    float* getData(int num_items,int &num);
    void writeToWave(float* data,float sr,int size,bool onlyfiltered);
    void writeToWave(float* data,char* filename,float sr,int size);
    char* getFilename();
    float* triangularMagnitude(float* data,int size);
private:
    float *buf1;
    bool onlyfiltered;
    char* filename1;
    int frame,samplerate,channel;
    char* filename;
    float* _triangularWave;
};

#endif // READINAUDIO_H
