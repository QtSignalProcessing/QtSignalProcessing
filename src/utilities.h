#ifndef UTILITIES_H
#define UTILITIES_H

class Utilities
{
  public:
    Utilities(float* data,int number);
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
};

#endif // UTILITIES_H
