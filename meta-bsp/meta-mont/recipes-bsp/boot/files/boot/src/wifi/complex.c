/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file complex.c
*   \brief  complex operation APIs.
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <math.h>
#include <complex.h>
#include <panther_debug.h>
#include <os_compat.h>

int printf(char *fmt, ...);

complex complex_add(complex a, complex b)
{
    complex result;

    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;

    return result;
}

complex complex_minus(complex a, complex b)
{
    complex result;

    result.real = a.real - b.real;
    result.imag = a.imag - b.imag;

    return result;
}

double complex_abs(complex a)
{
    return sqrt(a.real * a.real + a.imag * a.imag);
}

complex complex_multi(complex a, complex b)
{
    complex result;

    result.real = a.real*b.real - a.imag*b.imag;
    result.imag = a.real*b.imag + a.imag*b.real;

    return result;
}

complex complex_conj(complex a)
{
    complex result;

    result.real = a.real;
    result.imag = -1*a.imag;

    return result;
}

complex complex_div(complex a, complex b)
{
    complex result;

    result.real = (a.real*b.real + a.imag*b.imag)/(b.real*b.real + b.imag*b.imag);
    result.imag = (-1*a.real*b.imag + a.imag*b.real)/(b.real*b.real + b.imag*b.imag);

    return result;
}

complex complex_mean(complex *arr, int arr_len)
{
    complex *ptr = arr;
    complex result;
    int i;

    result.real=0; result.imag=0;

    for (i=0; i<arr_len; i++)       
    {
        result.real = result.real + ptr[i].real;
        result.imag = result.imag + ptr[i].imag;
    }
    result.real = result.real/arr_len;
    result.imag = result.imag/arr_len;

    return result;
}

double mean(double *arr, int arr_len)
{
    double *ptr = arr;
    double result=0;
    int i;

    for (i=0; i<arr_len; i++)       
    {
        result = result + (*(ptr++));
    }
    result = result/arr_len;

    return result;
}

int flt2hex(double x, int L)
{ 
    long int result;
    
    L = L-1; //shift from 0

    if (L > 32)
    {
        DBG_PRINTF(ERROR, "Exceed the maximum value of bits in floating exception.");
        return 0;
    }

    result = x*(2 << (L-1)); //double(int16(x*2^K-0.5)), K=L-1(start form 1)

    //boundary check
    if (result > 0)
    {
        if (result >= ((2<<(L-1))-1))
            result = (2<<(L-1))-1;
    }
    else if (result < 0)
    {
        result = result & ((2<<L)-1);
    }

    return result;
}

double hex2flt(int x, int L)
{ 
    double result = x;
 
    if ( x > ((2<<(L-2))-1) )
       result = x - (2<<(L-1));
 
    return result;
}

double max_double(double a, double b)
{
    return (a>b? a:b);
}

double abs_double(double a)
{
    return a<0? -a:a;
}

double __complex_to_db(complex *x)
{
    return (10*log10(((x->imag * x->imag) + (x->real * x->real))));
}

double abs_complex(complex *x)
{
	double real_abs = fabs(x->real);
	double imag_abs = fabs(x->imag);

	if(real_abs > imag_abs)
		return (real_abs + 0.5*imag_abs);
	else
		return (imag_abs + 0.5*real_abs);
}

double c_square(complex *c)
{
    return (((c->real) * (c->real)) + ((c->imag) * (c->imag)));
}


