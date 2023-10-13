#ifndef UTILITIES_H
#define UTILITIES_H

#include <QVector>

int gcd(int v1,int v2);


class Utilities
{
public:
    Utilities(QList<float>& data,int number);
    ~Utilities();
    void setBit(int bits);
    void setSampleFactor(int L,int down,int currentNumItems);
    //float* getQuantize();
    QList<float> getQuantize();
    double computeError();
    double getSNR(double varErr);
    void setReplace(bool replace);
    QList<float> getSampleData(bool org);
    void setFilterData(QList<float>);
    void updateUtilites(QList<float>,int number);
    int computeTrueBits();
    void setOnlyFiltered(bool onlyFiltered);
    const QList<float> getAmplitude(const QList<float>& in);
    const QList<float> getAmplitude();
    void setOrder(int order);
    QList<float> getFilterData();
    void setNyqFreq(double s);
    void butterOrCheby(bool i);
    void setRipple(float ripple);
    void setCutofFreq(QString s);
    void setFactor(float factor);
    double getNyq();
    QList<float> getOrgQuan();


private:
    int bits;
    int L;
    int down;
    QList<float> data;
    QList<float> qData;
    QList<float> sampleData;
    QList<float> filterData;
    float SNR;
    int number;
    int currentNumItems;
    bool replace;
    bool filtered;
    float getMax1(const QList<float>&,int num);
    int _trueBits;
    bool _butter;
    double _NyqFreq; //cutoff freq propotional to the Nyquist rate to avoid aliasing
    int _order; //order of the filter
    float _ripple;
    float _factor;
};

#endif // UTILITIES_H
