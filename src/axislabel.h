#ifndef AXISLABEL_H
#define AXISLABEL_H

class AxisLabel
{
  public:
    AxisLabel(double min,double max);
    void setCurrent(double currentMin,double currentMax);
    void autoScale();
    float* getLabels();
    float* getLabels(bool spec);
    void updateMaxMin(double max,double min);
    float* getYcord(float yscale,float vIn,float yshift,float vshift);
    float* getCord(float time, int number,float xScale,float xShift, double hshift ,int width,float hIn);
    ~AxisLabel();
     int size;
     float steps;
     void setMaxNumSteps(int maxNumSteps);

  private:
    double min;
    double max;
    int maxNumSteps;
    double currentMin;
    double currentMax;
    double range;
    int base;
    float* labels;
    void divideInterval();
    float* cord;
    float *yindex;
    float *ycord;
};

#endif // AXISLABEL_H
