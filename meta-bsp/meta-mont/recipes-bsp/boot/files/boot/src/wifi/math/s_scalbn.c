//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>   // Configuration header
#include "fdlibm.h"

#if 0
static const double
two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
huge   = 1.0e+300,
tiny   = 1.0e-300;
#endif

double scalbn (double x, int n)
{
	int  k,hx,lx;
	hx = CYG_LIBM_HI(x);
	lx = CYG_LIBM_LO(x);
	k = (hx&0x7ff00000)>>20;                /* extract exponent */
	if (k==0) {                             /* 0 or subnormal x */
		if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
		x *= two54; 
		hx = CYG_LIBM_HI(x);
		k = ((hx&0x7ff00000)>>20) - 54; 
		if (n< -50000) return tiny*x;       /*underflow*/
		}
	if (k==0x7ff) return x+x;               /* NaN or Inf */
	k = k+n; 
	if (k >  0x7fe) return huge*copysign(huge,x); /* overflow  */
	if (k > 0)                              /* normal result */
		{CYG_LIBM_HI(x) = (hx&0x800fffff)|(k<<20); return x;}
	if (k <= -54) {
		if (n > 50000)      /* in case integer overflow in n+k */
			return huge*copysign(huge,x);   /*overflow*/
		else return tiny*copysign(tiny,x);  /*underflow*/
	}
	k += 54;                                /* subnormal result */
	CYG_LIBM_HI(x) = (hx&0x800fffff)|(k<<20);
	return x*twom54;
}

//#endif 
