#ifndef UTILITIES_H
#define UTILITIES_H

#include <QVector>

class Utilities
{
  public:
    Utilities( float* data,int number);
    ~Utilities();
    void setBit(int bits);
    void setSampleFactor(int L,int down,int currentNumItems);
    float* getQuantize();
    double computeError();
    double getSNR(double varErr);
    void setReplace(bool replace);
    float* getSampleData(bool org);
    void setFilterData(float* filterData);
    void updateUtilites(float* data,int number);
    int computeTrueBits();
    void setOnlyFiltered(bool onlyFiltered);
    const QVector<float> getAmplitude(const QVector<float>& in);
    const QVector<float> getAmplitude();
    void setOrder(int order);
    QVector<float> getFilterData();
    void setNyqFreq(double s);
    void butterOrCheby(bool i);
    void setRipple(float ripple);
    void setCutofFreq(QString s);
    void setFactor(float factor);
    double getNyq();
    QVector<float> getOrgQuan();


  private:
    int bits;
    int L;
    int down;
    float* data;
    float* qData;
    float* sampleData;
    float* filterData;
    float SNR;
    int number;
    int currentNumItems;
    bool replace;
    bool filtered;
    float getMax1(float *a,int num);
    int _trueBits;
    bool _butter;
    double _NyqFreq; //cutoff freq propotional to the Nyquist rate to avoid aliasing
    int _order; //order of the filter
    float _ripple;
    float _factor;
};

#endif // UTILITIES_H
