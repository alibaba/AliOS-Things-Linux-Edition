//#ifdef CONFIG_MATH

//#include <os_compat.h>
#include <math.h>
#include "fdlibm.h"

double tan(double x)
{
	double y[2],z=0.0;
	int n, ix;

	/* High word of x. */
	ix = CYG_LIBM_HI(x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) return __kernel_tan(x,z,1);

	/* tan(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;            /* NaN */

	/* argument reduction needed */
	else {
		n = rem_pio2(x,y);
		return __kernel_tan(y[0],y[1],1-((n&1)<<1)); /*   1 -- n even
													-1 -- n odd */
	}
}

//#endif // CONFIG_MATH
