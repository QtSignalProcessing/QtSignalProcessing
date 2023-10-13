#include "chebyshevfilter.h"

#include <cmath>

cmplx czero = {0.0, 0.0};
//extern cmplx czero;
cmplx cone = {1.0, 0.0};
//extern cmplx cone;
double PI=3.14159265358979323846;
double MAXNUM =  1.79769313486231570815E308;
double MACHEP =  1.11022302462515654042E-16;   /* 2**-53 */
static double y[ARRSIZ];
static double zs[ARRSIZ];
cmplx z[ARRSIZ];
static double cbp = 0.0;
static double wc = 0.0;
static double c = 0.0;
static double cgam = 0.0;
static double scale = 0.0;
double dbfac = 0.0;
static double a = 0.0;
static double b = 0.0;
static double m = 0.0;
static double eps = 0.0;
static double rho = 0.0;
static double phi = 0.0;
static double cang = 0.0;
static double bw = 0.0;
static double ang = 0.0;
double fnyq = 0.0;
static double ai = 0.0;
static double pn = 0.0;
static double an = 0.0;
static double gam = 0.0;
static double cng = 0.0;
double gain = 0.0;
static int lr = 0;
static int i = 0;
static int j = 0;
static int jt = 0;
static int nc = 0;
static int ii = 0;
static int ir = 0;
int zord = 0;
static int icnt = 0;
static int mh = 0;
static int jj = 0;
static int jh = 0;
static int jl = 0;
static int n = 8;
static int np = 0;
static int nz = 0;
static int kind = 1;


ChebyshevFilter::ChebyshevFilter(int type,int order,double ripple,double samRate,double cutF1,double cutF2)
{
    aa = new double[ARRSIZ];
    pp = new double[ARRSIZ];
    dbfac = 10.0/log(10.0);
    kind=2;
    this->type=type;
    rn=order;
    n = rn;
    rn = n; /* ensure it is an integer */
    if(kind > 1) /* not Butterworth */
    {
        dbr=ripple;
        if(kind == 2)
        {
            /* For Chebyshev filter, ripples go from 1.0 to 1/sqrt(1+eps^2) */
            phi = exp(0.5*dbr/dbfac);

            if((n & 1) == 0)
                scale = phi;
            else
                scale = 1.0;
        }

   }
    fs=samRate;
    fnyq = 0.5 * fs;
    f2=cutF1;
    if((type & 1) == 0)
    {
        f1=cutF2;
        if((f1 <= 0.0) || (f1 >= fnyq))
            f1=0;
    }
    else
    {
        f1 = 0.0;
    }

    if( f2 < f1 )
    {
        a = f2;
        f2 = f1;
        f1 = a;
    }
    if( type == 3 ) /* high pass */
    {
        bw = f2;
        a = fnyq;
    }
    else
    {
        bw = f2 - f1;
        a = f2;
    }
   /* Frequency correspondence for bilinear transformation
    *
    *  Wanalog = tan( 2 pi Fdigital T / 2 )
    *
    * where T = 1/fs
    */
    ang = bw * PI / fs;
    cang = cos(ang);
    c = sin(ang) / cang; /* Wanalog */
    if(kind != 3)
    {
        wc = c;
    }

   /* Transformation from low-pass to band-pass critical frequencies
    *
    * Center frequency
    *                     cos( 1/2 (Whigh+Wlow) T )
    *  cos( Wcenter T ) = ----------------------
    *                     cos( 1/2 (Whigh-Wlow) T )
    *
    *
    * Band edges
    *            cos( Wcenter T) - cos( Wdigital T )
    *  Wanalog = -----------------------------------
    *                        sin( Wdigital T )
    */

    if(kind == 2)
    { /* Chebyshev */
        a = PI * (a+f1) / fs ;
        cgam = cos(a) / cang;
        a = 2.0 * PI * f2 / fs;
        cbp = (cgam - cos(a))/sin(a);
    }
    if( kind == 1 )
    { /* Butterworth */
        a = PI * (a+f1) / fs ;
        cgam = cos(a) / cang;
        a = 2.0 * PI * f2 / fs;
        cbp = (cgam - cos(a))/sin(a);
        scale = 1.0;
    }
    spln(); /* find s plane poles and zeros */
    zplna(); /* convert s plane to z plane */
    zplnb();
    zplnc();

}

ChebyshevFilter::~ChebyshevFilter()
{
    if(aa != nullptr)
       delete[] aa;
    if(pp != nullptr)
       delete[] pp;
}

void cadd(cmplx* a,cmplx* b,cmplx* c )
{
    c->r = b->r + a->r;
    c->i = b->i + a->i;
}

/* c = b - a */
void csub(cmplx *a, cmplx *b, cmplx *c )
{
    c->r = b->r - a->r;
    c->i = b->i - a->i;
}

/* c = b * a */
void cmul( cmplx *a, cmplx *b, cmplx *c )
{
    double y;
    y    = b->r * a->r  -  b->i * a->i;
    c->i = b->r * a->i  +  b->i * a->r;
    c->r = y;
}

/* c = b / a */
void cdiv( cmplx *a, cmplx *b, cmplx *c )
{
    double y, p, q, w;
    y = a->r * a->r  +  a->i * a->i;
    p = b->r * a->r  +  b->i * a->i;
    q = b->i * a->r  -  b->r * a->i;
    if( y < 1.0 )
    {
        w = MAXNUM * y;
        if((fabs(p) > w) || (fabs(q) > w) || (y == 0.0))
        {
            c->r = MAXNUM;
            c->i = MAXNUM;
            return;
        }
    }
    c->r = p/y;
    c->i = q/y;
}


/* b = a
   Caution, a `short' is assumed to be 16 bits wide.  */
void cmov( void *a, void *b )
{
    short *pa, *pb;
    int i;
    pa = (short *) a;
    pb = (short *) b;
    i = 8;
    do
        *pb++ = *pa++;
    while( --i );
}

void cneg( cmplx *a )
{
    a->r = -a->r;
    a->i = -a->i;
}

double cabs( cmplx *z )
{
    double x, y, b, re, im;
    int ex, ey, e;
    re = fabs( z->r );
    im = fabs( z->i );
    if( re == 0.0 )
        return( im );
    if( im == 0.0 )
        return( re );
    /* Get the exponents of the numbers */
    x = frexp( re, &ex );
    y = frexp( im, &ey );
    /* Check if one number is tiny compared to the other */
    e = ex - ey;
    if( e > PREC )
        return( re );
    if( e < -PREC )
        return( im );

    /* Find approximate exponent e of the geometric mean. */
    e = (ex + ey) >> 1;
    /* Rescale so mean is about 1 */
    x = ldexp( re, -e );
    y = ldexp( im, -e );
    /* Hypotenuse of the right triangle */
    b = sqrt( x * x  +  y * y );
    /* Compute the exponent of the answer. */
    y = frexp( b, &ey );
    ey = e + ey;
/* Check it for overflow and underflow. */
    if( ey > MAXEXP )
    {
        return( INFINITY );
    }
    if( ey < MINEXP )
        return(0.0);

    /* Undo the scaling */
    b = ldexp( b, e );
         return( b );
    }

void csqrt( cmplx *z, cmplx *w )
{
    cmplx q, s;
    double x, y, r, t;
    x = z->r;
    y = z->i;
    if( y == 0.0 )
    {
        if( x < 0.0 )
        {
        w->r = 0.0;
        w->i = sqrt(-x);
        return;
        }
        else
        {
        w->r = sqrt(x);
        w->i = 0.0;
        return;
        }
    }
    if( x == 0.0 )
    {
        r = fabs(y);
        r = sqrt(0.5*r);
        if( y > 0 )
        w->r = r;
        else
        w->r = -r;
        w->i = r;
        return;
    }

/* Approximate  sqrt(x^2+y^2) - x  =  y^2/2x - y^4/24x^3 + ... .
 * The relative error in the first term is approximately y^2/12x^2 .
 */
    if((fabs(y) < 2.e-4 * fabs(x)) && (x > 0))
    {
        t = 0.25*y*(y/x);
    }
    else
    {
        r = cabs(z);
        t = 0.5*(r - x);
    }

    r = sqrt(t);
    q.i = r;
    q.r = y/(2.0*r);
    /* Heron iteration in complex arithmetic */
    cdiv( &q, z, &s );
    cadd( &q, &s, w );
    w->r *= 0.5;
    w->i *= 0.5;
}

/* calculate s plane poles and zeros, normalized to wc = 1 */
int ChebyshevFilter::spln()
{
    for(i = 0; i < ARRSIZ; i++)
        zs[i] = 0.0;
    np = (n+1)/2;
    nz = 0;
    if(kind == 1)
    {
        /* Butterworth poles equally spaced around the unit circle*/
        if( n & 1 )
            m = 0.0;
        else
            m = PI / (2.0*n);
        for( i=0; i<np; i++ )
        { /* poles */
            lr = i + i;
            zs[lr] = -cos(m);
            zs[lr+1] = sin(m);
            m += PI / n;
        }
        /* high pass or band reject*/
        if( type >= 3 )
        {
            /* map s => 1/s*/
            for( j=0; j<np; j++ )
            {
                ir = j + j;
                ii = ir + 1;
                b = zs[ir]*zs[ir] + zs[ii]*zs[ii];
                zs[ir] = zs[ir] / b;
                zs[ii] = zs[ii] / b;
            }
        /* The zeros at infinity map to the origin.*/
        nz = np;
        if( type == 4 )
        {
            nz += n/2;
        }
        for( j=0; j<nz; j++ )
        {
            ir = ii + 1;
            ii = ir + 1;
            zs[ir] = 0.0;
            zs[ii] = 0.0;
        }
    }
}
    if( kind == 2 )
    {
        rho = (phi - 1.0)*(phi+1);  /* rho = eps^2 = {sqrt(1+eps^2)}^2 - 1 */
        eps = sqrt(rho);
/* sqrt( 1 + 1/eps^2 ) + 1/eps  = {sqrt(1 + eps^2)  +  1} / eps
*/
        phi = (phi + 1.0) / eps;
        phi = pow( phi, 1.0/rn );  /* raise to the 1/n power */
        b = 0.5 * (phi + 1.0/phi); /* y coordinates are on this circle */
        a = 0.5 * (phi - 1.0/phi); /* x coordinates are on this circle */
        if(n & 1)
            m = 0.0;
        else
            m = PI / (2.0*n);
        for( i=0; i<np; i++ )
        { /* poles */
            lr = i + i;
            zs[lr] = -a * cos(m);
            zs[lr+1] = b * sin(m);
            m += PI / n;
        }
        /* high pass or band reject*/
        if( type >= 3 )
        {
            /* map s => 1/s*/
            for( j=0; j<np; j++ )
            {
                ir = j + j;
                ii = ir + 1;
                b = zs[ir]*zs[ir] + zs[ii]*zs[ii];
                zs[ir] = zs[ir] / b;
                zs[ii] = zs[ii] / b;
            }
            /* The zeros at infinity map to the origin.*/
            nz = np;
            if( type == 4 )
            {
                nz += n/2;
            }
            for( j=0; j<nz; j++ )
            {
                ir = ii + 1;
                ii = ir + 1;
                zs[ir] = 0.0;
                zs[ii] = 0.0;
            }
        }
    }
    j = 0;
    for(i=0; i<np+nz; i++)
    {
        a = zs[j];
        ++j;
        b = zs[j];
        ++j;
        if(i>9)
            break;
    }
    return 0;
}

int ChebyshevFilter::zplna()
{
    cmplx r, cnum, cden, cwc, ca, cb, b4ac;
    double C;
    if( kind == 3 )
        C = c;
    else
    C = wc;
    for( i=0; i<ARRSIZ; i++ )
    {
        z[i].r = 0.0;
        z[i].i = 0.0;
    }
    nc = np;
    jt = -1;
    ii = -1;
    for( icnt=0; icnt<2; icnt++ )
    {
    /* The maps from s plane to z plane */
        do
        {
            ir = ii + 1;
            ii = ir + 1;
            r.r = zs[ir];
            r.i = zs[ii];
            switch( type )
            {
                case 1:
                case 3:
                /* Substitute  s - r  =  s/wc - r = (1/wc)(z-1)/(z+1) - r
                 *
                 *     1  1 - r wc (       1 + r wc )
                 * =  --- -------- ( z  -  -------- )
                 *    z+1    wc    (       1 - r wc )
                 *
                 * giving the root in the z plane.
                 */
                    cnum.r = 1 + C * r.r;
                    cnum.i = C * r.i;
                    cden.r = 1 - C * r.r;
                    cden.i = -C * r.i;
                    jt += 1;
                    cdiv( &cden, &cnum, &z[jt] );
                    if( r.i != 0.0 )
                    {
                    /* fill in complex conjugate root */
                        jt += 1;
                        z[jt].r = z[jt-1 ].r;
                        z[jt].i = -z[jt-1 ].i;
                    }
                    break;
                case 2:
                case 4:
                /* Substitute  s - r  =>  s/wc - r
                 *
                 *     z^2 - 2 z cgam + 1
                 * =>  ------------------  -  r
                 *         (z^2 + 1) wc
                 *
                 *         1
                 * =  ------------  [ (1 - r wc) z^2  - 2 cgam z  +  1 + r wc ]
                 *    (z^2 + 1) wc
                 *
                 * and solve for the roots in the z plane.
                 */
                    if( kind == 2 )
                        cwc.r = cbp;
                    else
                        cwc.r = c;
                    cwc.i = 0.0;
                    cmul( &r, &cwc, &cnum );     /* r wc */
                    csub( &cnum, &cone, &ca );   /* a = 1 - r wc */
                    cmul( &cnum, &cnum, &b4ac ); /* 1 - (r wc)^2 */
                    csub( &b4ac, &cone, &b4ac );
                    b4ac.r *= 4.0;               /* 4ac */
                    b4ac.i *= 4.0;
                    cb.r = -2.0 * cgam;          /* b */
                    cb.i = 0.0;
                    cmul( &cb, &cb, &cnum );     /* b^2 */
                    csub( &b4ac, &cnum, &b4ac ); /* b^2 - 4 ac */
                    csqrt( &b4ac, &b4ac );
                    cb.r = -cb.r;  /* -b */
                    cb.i = -cb.i;
                    ca.r *= 2.0; /* 2a */
                    ca.i *= 2.0;
                    cadd( &b4ac, &cb, &cnum );   /* -b + sqrt( b^2 - 4ac) */
                    cdiv( &ca, &cnum, &cnum );   /* ... /2a */
                    jt += 1;
                    cmov( &cnum, &z[jt] );
                    if( cnum.i != 0.0 )
                    {
                        jt += 1;
                        z[jt].r = cnum.r;
                        z[jt].i = -cnum.i;
                    }
                    if((r.i != 0.0) || (cnum.i == 0))
                    {
                        csub( &b4ac, &cb, &cnum );  /* -b - sqrt( b^2 - 4ac) */
                        cdiv( &ca, &cnum, &cnum );  /* ... /2a */
                        jt += 1;
                        cmov( &cnum, &z[jt] );
                        if( cnum.i != 0.0 )
                        {
                            jt += 1;
                            z[jt].r = cnum.r;
                            z[jt].i = -cnum.i;
                        }
                    }
            } /* end switch */
        }
        while( --nc > 0 );

    if( icnt == 0 )
    {
        zord = jt+1;
        if( nz <= 0 )
        {
            if( kind != 3 )
                return(0);
            else
                break;
        }
    }
    nc = nz;
    } /* end for() loop */
    return 0;
}

int ChebyshevFilter::zplnb()
{
    if(kind != 3)
    { /* Butterworth or Chebyshev */
        /* generate the remaining zeros */
        while( 2*zord - 1 > jt )
        {
            if( type != 3 )
            {
                jt += 1;
                z[jt].r = -1.0; /* zero at Nyquist frequency */
                z[jt].i = 0.0;
            }
            if( (type == 2) || (type == 3) )
            {
                jt += 1;
                z[jt].r = 1.0; /* zero at 0 Hz */
                z[jt].i = 0.0;
            }
        }
    }
    /* Expand the poles and zeros into numerator and
     * denominator polynomials
     */
    for( icnt=0; icnt<2; icnt++ )
    {
        for( j=0; j<ARRSIZ; j++ )
        {
            pp[j] = 0.0;
            y[j] = 0.0;
        }
        pp[0] = 1.0;
        for( j=0; j<zord; j++ )
        {
            jj = j;
            if( icnt )
                jj += zord;
            a = z[jj].r;
            b = z[jj].i;
            for( i=0; i<=j; i++ )
            {
                jh = j - i;
                pp[jh+1] = pp[jh+1] - a * pp[jh] + b * y[jh];
                y[jh+1] =  y[jh+1]  - b * pp[jh] - a * y[jh];
            }
        }
        if( icnt == 0 )
        {
            for( j=0; j<=zord; j++ )
                aa[j] = pp[j];
        }
    }
        /* Scale factors of the pole and zero polynomials */
       a = 1.0;
    switch( type )
    {
        case 3:
            a = -1.0;
        case 1:
        case 4:
            pn = 1.0;
            an = 1.0;
        for( j=1; j<=zord; j++ )
        {
            pn = a * pn + pp[j];
            an = a * an + aa[j];
        }
            break;
        case 2:
            gam = PI/2.0 - asin( cgam );  /* = acos( cgam ) */
            mh = zord/2;
            pn = pp[mh];
            an = aa[mh];
            ai = 0.0;
            if( mh > ((zord/4)*2) )
            {
                ai = 1.0;
                pn = 0.0;
                an = 0.0;
            }
            for( j=1; j<=mh; j++ )
            {
                a = gam * j - ai * PI / 2.0;
                cng = cos(a);
                jh = mh + j;
                jl = mh - j;
                pn = pn + cng * (pp[jh] + (1.0 - 2.0 * ai) * pp[jl]);
                an = an + cng * (aa[jh] + (1.0 - 2.0 * ai) * aa[jl]);
            }
    }
    return 0;
}

int ChebyshevFilter::zplnc()
{
    gain = an/(pn*scale);
    if((kind != 3) && (pn == 0))
        gain = 1.0;
    for(j=0; j<=zord; j++)
        pp[j] = gain * pp[j];
    return 0;
}
