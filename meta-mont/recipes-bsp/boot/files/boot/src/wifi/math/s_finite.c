//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>   // Configuration header
#include "fdlibm.h"

int finite(double x)
{
	int hx; 
	hx = CYG_LIBM_HI(x);
	return  (unsigned)((hx&0x7fffffff)-0x7ff00000)>>31;
}

//#endif // CONFIG_MATH
