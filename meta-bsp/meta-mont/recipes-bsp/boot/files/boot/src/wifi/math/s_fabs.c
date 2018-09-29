//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>   // Configuration header
#include "fdlibm.h"

double fabs(double x)
{
	CYG_LIBM_HI(x) &= 0x7fffffff;
	return x;
}

//#endif // CONFIG_MATH
