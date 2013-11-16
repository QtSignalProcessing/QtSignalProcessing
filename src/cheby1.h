#ifndef CHEBY1_H
#define CHEBY1_H

#ifndef INFINITY
#define INFINITY 1.79769313486231570815E308
#endif

#define PREC 27
#define MAXEXP 1024
#define MINEXP -1077
#define ARRSIZ 50

class cheby1
{

  public:
    cheby1(int type,int order,double ripple,double samRate,double cutF1,double cutF2);
   // ~cheby1();
     double rn;
     double dbr;
     int type;
     double fs;
     double f1;
     double f2;
     int spln();
     int zplna();
     int zplnb();
     int zplnc();
     double aa[ARRSIZ];
     double pp[ARRSIZ];
};

#endif // CHEBY1_H
