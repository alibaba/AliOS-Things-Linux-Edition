//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>   // Configuration header
#include "fdlibm.h"

double cos(double x)
{
	double y[2],z=0.0;
	int n, ix;

	/* High word of x. */
	ix = CYG_LIBM_HI(x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) return __kernel_cos(x,z);

	/* cos(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;

	/* argument reduction needed */
	else {
		n = rem_pio2(x,y);
		switch(n&3) {
			case 0: return  __kernel_cos(y[0],y[1]);
			case 1: return -__kernel_sin(y[0],y[1],1);
			case 2: return -__kernel_cos(y[0],y[1]);
			default:
					return  __kernel_sin(y[0],y[1],1);
		}
	}
}

//#endif // CONFIG_MATH
