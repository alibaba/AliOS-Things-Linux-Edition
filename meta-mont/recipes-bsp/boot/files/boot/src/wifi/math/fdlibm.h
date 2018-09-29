//#ifdef CONFIG_MATH

#ifndef __FDLIBM_H__
#define __FDLIBM_H__
#include <stdint.h>
typedef union 
{
    double value;
    
    struct 
    {
#if defined(BIG_ENDIAN)
        unsigned int msw;
        unsigned int lsw;
#else
        unsigned int lsw;
        unsigned int msw;
#endif
    } parts;

} Cyg_libm_ieee_double_shape_type;

typedef int32_t 	__int32_t;
typedef uint32_t 	__uint32_t;

// MACRO DEFINITIONS

#define CYG_LIBM_HI(__x)  (((Cyg_libm_ieee_double_shape_type *)&__x)->parts.msw)
#define CYG_LIBM_LO(__x)  (((Cyg_libm_ieee_double_shape_type *)&__x)->parts.lsw)
#define CYG_LIBM_HIp(__x) (((Cyg_libm_ieee_double_shape_type *)__x)->parts.msw)
#define CYG_LIBM_LOp(__x) (((Cyg_libm_ieee_double_shape_type *)__x)->parts.lsw)


/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)                                \
do {                                                            \
  Cyg_libm_ieee_double_shape_type ew_u;                         \
  ew_u.value = (d);                                             \
  (ix0) = ew_u.parts.msw;                                       \
  (ix1) = ew_u.parts.lsw;                                       \
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
  Cyg_libm_ieee_double_shape_type gh_u;                         \
  gh_u.value = (d);                                             \
  (i) = gh_u.parts.msw;                                         \
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)                                       \
do {                                                            \
  Cyg_libm_ieee_double_shape_type gl_u;                         \
  gl_u.value = (d);                                             \
  (i) = gl_u.parts.lsw;                                         \
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d,ix0,ix1)                                 \
do {                                                            \
  Cyg_libm_ieee_double_shape_type iw_u;                         \
  iw_u.parts.msw = (ix0);                                       \
  iw_u.parts.lsw = (ix1);                                       \
  (d) = iw_u.value;                                             \
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)                                      \
do {                                                            \
  Cyg_libm_ieee_double_shape_type sh_u;                         \
  sh_u.value = (d);                                             \
  sh_u.parts.msw = (v);                                         \
  (d) = sh_u.value;                                             \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d,v)                                       \
do {                                                            \
  Cyg_libm_ieee_double_shape_type sl_u;                         \
  sl_u.value = (d);                                             \
  sl_u.parts.lsw = (v);                                         \
  (d) = sl_u.value;                                             \
} while (0)

/* define constant */

#define zero		0.0
#define one			1.0
#define two			2.0
/* 0x41700000, 0x00000000 */
#define two24		1.67772160000000000000e+07
/* 0x3E700000, 0x00000000 */
#define twon24		5.96046447753906250000e-08 
/* 0x43400000, 0x00000000 */
#define two53		9007199254740992.0	
/* 0x43500000, 0x00000000 */
#define two54		1.80143985094819840000e+16 
/* 0x3C900000, 0x00000000 */
#define twom54		5.55111512312578270212e-17 
#define huge		1.000e+300
#define tiny		1.0e-300
/* 0x3FE00000, 0x00000000 */
#define half		5.00000000000000000000e-01 

/* 0x3FA55555, 0x5555554C */
#define C1			4.16666666666666019037e-02 
/* 0xBF56C16C, 0x16C15177 */
#define C2			-1.38888888888741095749e-03 
/* 0x3EFA01A0, 0x19CB1590 */
#define C3			2.48015872894767294178e-05 
/* 0xBE927E4F, 0x809C52AD */
#define C4			-2.75573143513906633035e-07 
/* 0x3E21EE9E, 0xBDB4B1C4 */
#define C5			2.08757232129817482790e-09
/* 0xBDA8FAE9, 0xBE8838D4 */
#define C6			-1.13596475577881948265e-11 
/* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
#define cp			9.61796693925975554329e-01 
/* 0x3FEEC709, 0xE0000000 =(float)cp */
#define cp_h		9.61796700954437255859e-01 
/* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
#define cp_l		-7.02846165095275826516e-09 

/* 0x3FE45F30, 0x6DC9C883 */
#define invpio2		6.36619772367581382433e-01 
/* 0x3FDBCB7B, 0x1526E50E */
#define ivln10		4.34294481903251816668e-01 
/* 0x3FF71547, 0x652B82FE =1/ln2 */
#define ivln2		1.44269504088896338700e+00 
/* 0x3FF71547, 0x60000000 =24b 1/ln2*/
#define ivln2_h		1.44269502162933349609e+00 
/* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/
#define ivln2_l		1.92596299112661746887e-08 

/* 0x3FD34413, 0x509F6000 */
#define log10_2hi	3.01029995663611771306e-01 
/* 0x3D59FEF3, 0x11F12B36 */
#define log10_2lo	3.69423907715893078616e-13 

/* 0x3FE33333, 0x33333303 */
#define L1			5.99999999999994648725e-01 
/* 0x3FDB6DB6, 0xDB6FABFF */
#define L2			4.28571428578550184252e-01
/* 0x3FD55555, 0x518F264D */
#define L3			3.33333329818377432918e-01 
/* 0x3FD17460, 0xA91D4101 */
#define L4			2.72728123808534006489e-01 
/* 0x3FCD864A, 0x93C9DB65 */
#define L5			2.30660745775561754067e-01 
/* 0x3FCA7E28, 0x4A454EEF */
#define L6			2.06975017800338417784e-01 

/* 3fe62e42 fee00000 */
#define ln2_hi		6.93147180369123816490e-01 
/* 3dea39ef 35793c76 */
#define ln2_lo		1.90821492927058770002e-10  
/* 3FE55555 55555593 */
#define Lg1			6.666666666666735130e-01  
/* 3FD99999 9997FA04 */
#define Lg2			3.999999999940941908e-01  
/* 3FD24924 94229359 */
#define Lg3			2.857142874366239149e-01  
/* 3FCC71C5 1D8E78AF */
#define Lg4			2.222219843214978396e-01  
/* 3FC74664 96CB03DE */
#define Lg5			1.818357216161805012e-01  
/* 3FC39A09 D078C69F */
#define Lg6			1.531383769920937332e-01  
/* 3FC2F112 DF3E5244 */
#define Lg7			1.479819860511658591e-01  

/* 0x3FE62E42, 0xFEFA39EF */
#define lg2			6.93147180559945286227e-01 
/* 0x3FE62E43, 0x00000000 */
#define lg2_h		6.93147182464599609375e-01 
/* 0xBE205C61, 0x0CA86C39 */
#define lg2_l		-1.90465429995776804525e-09 

/* -(1024-log2(ovfl+.5ulp)) */
#define ovt			8.0085662595372944372e-0017 

/* 0x3FC55555, 0x5555553E */
#define P1			1.66666666666666019037e-01 
/* 0xBF66C16C, 0x16BEBD93 */
#define P2			-2.77777777770155933842e-03 
/* 0x3F11566A, 0xAF25DE2C */
#define P3			6.61375632143793436117e-05
/* 0xBEBBBD41, 0xC5D26BF1 */
#define P4			-1.65339022054652515390e-06 
/* 0x3E663769, 0x72BEA4D0 */
#define P5			4.13813679705723846039e-08 

/* 0x3FF921FB, 0x54400000 */
#define pio2_1		1.57079632673412561417e+00 
/* 0x3DD0B461, 0x1A626331 */
#define pio2_1t		6.07710050650619224932e-11 
/* 0x3DD0B461, 0x1A600000 */
#define pio2_2		6.07710050630396597660e-11 
/* 0x3BA3198A, 0x2E037073 */
#define pio2_2t		2.02226624879595063154e-21 
/* 0x3BA3198A, 0x2E000000 */
#define pio2_3		2.02226624871116645580e-21 
/* 0x397B839A, 0x252049C1 */
#define pio2_3t		8.47842766036889956997e-32 
/* 0x3FF921FB, 0x54442D18 */
#define	pio2_hi		1.57079632679489655800e+00 
/* 0x3C91A626, 0x33145C07 */
#define pio2_lo		6.12323399573676603587e-17
/* 0x3FE921FB, 0x54442D18 */
#define pio4		7.85398163397448278999e-01 
/* 0x3FE921FB, 0x54442D18 */
#define pio4_hi		7.85398163397448278999e-01
/* 0x3C81A626, 0x33145C07 */
#define pio4lo		3.06161699786838301793e-17 
/* 0x3FC55555, 0x55555555 */
#define pS0			1.66666666666666657415e-01 
/* 0xBFD4D612, 0x03EB6F7D */
#define pS1			-3.25565818622400915405e-01 
/* 0x3FC9C155, 0x0E884455 */
#define pS2			2.01212532134862925881e-01
/* 0xBFA48228, 0xB5688F3B */
#define pS3			-4.00555345006794114027e-02 
/* 0x3F49EFE0, 0x7501B288 */
#define pS4			7.91534994289814532176e-04 
/* 0x3F023DE1, 0x0DFDF709 */
#define pS5			3.47933107596021167570e-05 

/* 0xC0033A27, 0x1C8A2D4B */
#define qS1			-2.40339491173441421878e+00
/* 0x40002AE5, 0x9C598AC8 */
#define qS2			2.02094576023350569471e+00 
/* 0xBFE6066C, 0x1B8D0159 */
#define qS3			-6.88283971605453293030e-01 
/* 0x3FB3B8C5, 0xB12E9282 */
#define qS4			7.70381505559019352791e-02 

/* 0xBFC55555, 0x55555549 */
#define S1			-1.66666666666666324348e-01 
/* 0x3F811111, 0x1110F8A6 */
#define S2			8.33333333332248946124e-03 
/* 0xBF2A01A0, 0x19C161D5 */
#define S3			-1.98412698298579493134e-04 
/* 0x3EC71DE3, 0x57B1FE7D */
#define S4			2.75573137070700676789e-06 
/* 0xBE5AE5E6, 0x8A2B9CEB */
#define S5			-2.50507602534068634195e-08 
/* 0x3DE5D93A, 0x5ACFD57C */
#define S6			1.58969099521155010221e-10

double __kernel_sin( double, double, int ); // __attribute__ ((section(".rfc")));

double __kernel_cos( double, double ); // __attribute__ ((section(".rfc")));

double __kernel_tan( double, double, int ); // __attribute__ ((section(".rfc")));

int __kernel_rem_pio2( double *, double *, int, int, int, const int * ); // __attribute__ ((section(".rfc")));

#endif // __FDLIBM_H__

//#endif
