#ifndef CHEBY1_H
#define CHEBY1_H

#ifndef INFINITY
#define INFINITY 1.79769313486231570815E308
#endif

#define PREC 27
#define MAXEXP 1024
#define MINEXP -1077
#define ARRSIZ 50

typedef struct
{
    double r;
    double i;
} cmplx;



double cabs ( cmplx *z );
void cadd ( cmplx *a, cmplx *b, cmplx *c );
void cdiv ( cmplx *a, cmplx *b, cmplx *c );
void cmov ( void *a, void *b );
void cmul ( cmplx *a, cmplx *b, cmplx *c );
void cneg ( cmplx *a );
void csqrt ( cmplx *z, cmplx *w );
void csub ( cmplx *a, cmplx *b, cmplx *c );
int zplnc ( void );

class ChebyshevFilter
{
  public:
    ChebyshevFilter(int type,int order,double ripple,double samRate,double cutF1,double cutF2);
    ~ChebyshevFilter();
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
    double* aa;
    double* pp;


};

#endif // CHEBY1_H
