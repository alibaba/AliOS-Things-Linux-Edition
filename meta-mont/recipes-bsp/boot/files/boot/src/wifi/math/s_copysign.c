//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>
#include "fdlibm.h"

double copysign(double x, double y)
{
	CYG_LIBM_HI(x) = (CYG_LIBM_HI(x)&0x7fffffff)|(CYG_LIBM_HI(y)&0x80000000);
	return x;
}

//#endif // CONFIG_MATH
