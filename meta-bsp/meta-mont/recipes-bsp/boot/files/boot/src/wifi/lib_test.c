//#include <linux/kernel.h>
//#include <linux/init.h>
#include <os_compat.h>
#include <panther_debug.h>
#include "math.h"
#include <rfc_comm.h>
#include <lib_test.h>
//#define DBG             printk
#ifdef DRAGONITE_RFC_DEBUG
#define DBG_DOUBLE(x)   dbg_double(RFC_DBG_LEVEL, x)
#else
#define DBG_DOUBLE(x)
#endif

//void dbg_double(int level, double x)
//{
//    if(isnan(x))
//    {
//        DBG_PRINTF(INFO_NO_PREFIX,  "   NaN   ");
//    }
//    else if(!finite(x))
//    {
//        DBG_PRINTF(INFO_NO_PREFIX,  "   Inf   ");
//    }
//    else if(x>=0)
//    {
//        DBG_PRINTF(INFO_NO_PREFIX, "%d.%08d", (int) x, (int) ((x - ((int) x)) * 100000000));
//    }
//    else
//    {
//        x = -1 * x;
//        DBG_PRINTF(INFO_NO_PREFIX, "-%d.%08d", (int) x, (int) ((x - ((int) x)) * 100000000));
//    }
//}

int camelot_lib_test(void)
{
    int i;

    double x;
    double y;

    for(i=-10;i<=10;i++)
    {
        x = 0.9 * i;
    
        y = fabs(x);
        DBG("\nfabs ");    DBG_DOUBLE(y);
    
        y = asin(x);
        DBG("\nasin ");    DBG_DOUBLE(y);
    
        y = sqrt(x);
        DBG("\nsqrt ");    DBG_DOUBLE(y);
    
        y = cos(x);
        DBG("\ncos ");    DBG_DOUBLE(y);
    
        y = tan(x);
        DBG("\ntan ");    DBG_DOUBLE(y);
    
        y = sin(x);
        DBG("\nsin ");    DBG_DOUBLE(y);
    
        y = pow(x, x);
        DBG("\npow ");    DBG_DOUBLE(y);
    
        y = log10(x);
        DBG("\nlog10 ");    DBG_DOUBLE(y);
    
        DBG("\n");
    }

    return 0;
}

/* Test results

fabs 9.000000
asin nan
sqrt nan
cos -0.911130
tan 0.452316
sin -0.412118
pow -0.000000
log10 nan

fabs 8.100000
asin nan
sqrt nan
cos -0.243544
tan 3.982398
sin -0.969890
pow nan
log10 nan

fabs 7.200000
asin nan
sqrt nan
cos 0.608351
tan -1.304621
sin -0.793668
pow nan
log10 nan

fabs 6.300000
asin nan
sqrt nan
cos 0.999859
tan -0.016816
sin -0.016814
pow nan
log10 nan

fabs 5.400000
asin nan
sqrt nan
cos 0.634693
tan 1.217541
sin 0.772764
pow nan
log10 nan

fabs 4.500000
asin nan
sqrt nan
cos -0.210796
tan -4.637332
sin 0.977530
pow nan
log10 nan

fabs 3.600000
asin nan
sqrt nan
cos -0.896758
tan -0.493467
sin 0.442520
pow nan
log10 nan

fabs 2.700000
asin nan
sqrt nan
cos -0.904072
tan 0.472728
sin -0.427380
pow nan
log10 nan

fabs 1.800000
asin nan
sqrt nan
cos -0.227202
tan 4.286262
sin -0.973848
pow nan
log10 nan

fabs 0.900000
asin -1.119770
sqrt nan
cos 0.621610
tan -1.260158
sin -0.783327
pow nan
log10 nan

fabs 0.000000
asin 0.000000
sqrt 0.000000
cos 1.000000
tan 0.000000
sin 0.000000
pow 1.000000
log10 -inf

fabs 0.900000
asin 1.119770
sqrt 0.948683
cos 0.621610
tan 1.260158
sin 0.783327
pow 0.909533
log10 -0.045757

fabs 1.800000
asin nan
sqrt 1.341641
cos -0.227202
tan -4.286262
sin 0.973848
pow 2.880650
log10 0.255273

fabs 2.700000
asin nan
sqrt 1.643168
cos -0.904072
tan -0.472728
sin 0.427380
pow 14.611075
log10 0.431364

fabs 3.600000
asin nan
sqrt 1.897367
cos -0.896758
tan 0.493467
sin -0.442520
pow 100.621087
log10 0.556303

fabs 4.500000
asin nan
sqrt 2.121320
cos -0.210796
tan 4.637332
sin -0.977530
pow 869.873923
log10 0.653213

fabs 5.400000
asin nan
sqrt 2.323790
cos 0.634693
tan -1.217541
sin -0.772764
pow 9014.181598
log10 0.732394

fabs 6.300000
asin nan
sqrt 2.509980
cos 0.999859
tan 0.016816
sin 0.016814
pow 108603.910096
log10 0.799341

fabs 7.200000
asin nan
sqrt 2.683282
cos 0.608351
tan 1.304621
sin 0.793668
pow 1488654.704749
log10 0.857332

fabs 8.100000
asin nan
sqrt 2.846050
cos -0.243544
tan -3.982398
sin 0.969890
pow 22841712.078457
log10 0.908485

fabs 9.000000
asin nan
sqrt 3.000000
cos -0.911130
tan -0.452316
sin 0.412118
pow 387420489.000000
log10 0.954243

*/

//module_init(camelot_lib_test);

