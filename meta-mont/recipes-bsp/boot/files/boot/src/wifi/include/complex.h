#ifndef __COMPLEX_H__
#define __COMPLEX_H__

typedef struct {
    double real, imag;
} complex;

complex complex_add(complex a, complex b);
complex complex_minus(complex a, complex b);
double complex_abs(complex a);
complex complex_multi(complex a, complex b);
complex complex_conj(complex a);
complex complex_div(complex a, complex b);
complex complex_mean(complex *arr, int arr_len);
double mean(double *arr, int arr_len);
int flt2hex(double x, int L);
double hex2flt(int x, int L);
double max_double(double a, double b);
double abs_double(double a);
double __complex_to_db(complex *x);
double abs_complex(complex *x);
double c_square(complex *c);

#endif //__COMPLEX_H__
