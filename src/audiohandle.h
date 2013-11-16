#ifndef AUDIOHANDLE_H
#define AUDIOHANDLE_H

#include "sndfile.h"

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
    void writeToWave(float* data,char* filename,float sr,int size);
    float* triangularMagnitude(float* data,int size);

  private:
    float *buf1;
    int frame,samplerate,channel;
    float* _triangularWave;
};

#endif // READINAUDIO_H
