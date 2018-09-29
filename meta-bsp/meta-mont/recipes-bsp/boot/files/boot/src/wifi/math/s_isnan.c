//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>  // Configuration header
#include "fdlibm.h"

int isnan(double x)
{
	int hx,lx;
	hx = (CYG_LIBM_HI(x)&0x7fffffff);
	lx = CYG_LIBM_LO(x);
	hx |= (unsigned)(lx|(-lx))>>31; 
	hx = 0x7ff00000 - hx;
	return ((unsigned)(hx))>>31;
}

//#endif // CONFIG_MATH
