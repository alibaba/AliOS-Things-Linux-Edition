/*---------------------------------------------------------------
  The math.h & log.c & pow.c are modified from ecos2.0/package. 
---------------------------------------------------------------*/

//#include <cyg/infra/cyg_type.h>         // types & externC

#ifndef __LIB_MATH__
#define __LIB_MATH__

/* has been referenced by other func */
double log(double x);
double pow(double x, double y);
double scalbn (double x, int n);
double copysign(double x, double y);

/* fill to _init (for reduce rfc runtime size) */
int finite(double x); // __attribute__ ((section(".rfc")));
int isnan(double x); // __attribute__ ((section(".rfc")));
int rem_pio2(double x, double *y); // __attribute__ ((section(".rfc")));
double log10(double x); // __attribute__ ((section(".rfc")));
double sqrt(double x); // __attribute__ ((section(".rfc")));
double fabs(double x); // __attribute__ ((section(".rfc")));
double floor(double x); // __attribute__ ((section(".rfc")));
double ceil(double x); // __attribute__ ((section(".rfc")));
double sin(double x); // __attribute__ ((section(".rfc")));
double tan(double x); // __attribute__ ((section(".rfc")));
double cos(double x); // __attribute__ ((section(".rfc")));
double asin(double x); // __attribute__ ((section(".rfc")));

#endif // __LIB_MATH__

