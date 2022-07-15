/*
 ===============================================================================
 |                         Some utility math functions                         |
 ===============================================================================
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef SPM_H
#define SPM_H

#include <math.h>

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
/* = Floating point handling = */
float
spm_round_off_d(float num, unsigned int round_off_c);
/*
 * Round off to a decimal point with proper 5-even/odd rule.
 *
 * About 5-even/odd rule:
 * https://students.flinders.edu.au/content/dam/student/slc/rounding-numbers.pdf
 */

float
spm_signifi_d(float num, unsigned int signifi_c);
/* Return the float number with the given significant count. */

int
spm_is_equal_deci(float num1, float num2, unsigned int precision);
/* Check if two float numbers are equal till a precision in decimal places. */

int
spm_is_equal_signi(float num1, float num2, unsigned int precision);
/* Check if two float numbers are equal till a precision in significant digits. */

/* = Polynomial = */
float
spm_poly_val_point(unsigned int poly_degree, float *poly_body, float point);
/* Return the value of the polynomial at the given point. */

#endif /* SPM_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef SPM_IMPLEMENTATION
#define SPM_IMPLEMENTED

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */
/* = Floating point handling = */
float
spm_round_off_d(float num, unsigned int round_off_c)
{
	int mul_factor     = pow(10, round_off_c);
	int num_multiplied = num * mul_factor * 10;
	int last_decimal   = num_multiplied % 10;
	num_multiplied /= 10;
	int second_last_decimal = num_multiplied % 10;

	/* Special case when the last decimal is 5 AND the second last decimal
	 * is even. */
	if (last_decimal == 5 || last_decimal == -5)
		if (second_last_decimal % 2 == 0) {
			if (num > 0)
				return floorf(num * mul_factor) / mul_factor;
			return ceilf(num * mul_factor) / mul_factor;
		}

	/* Else return nearest round off. */
	return roundf(num * mul_factor) / mul_factor;
}

float
spm_signifi_d(float num, unsigned int signifi_c)
{
	/* Get the number of digits in the whole number part. */
	int num_int = fabs(num), whole_num_c = 1;
	while (num_int > 9) {
		num_int /= 10;
		whole_num_c++;
	}

	/* Perform the round off now. */
	return spm_round_off_d(num, signifi_c - whole_num_c);
}

int
spm_is_equal_deci(float num1, float num2, unsigned int precision)
{
	int factor = pow(10, precision);
	return (int)(num1 * factor) == (int)(num2 * factor);
}

int
spm_is_equal_signi(float num1, float num2, unsigned int precision)
{
	/* Get the number of digits in the whole number part. */
	int num_int = fabs(num2), whole_num_c = 1;
	while (num_int > 9) {
		num_int /= 10;
		whole_num_c++;
	}

	int factor = pow(10, precision - whole_num_c);
	int a      = (int)(num1 * factor);
	int b      = (int)(num2 * factor);
	return a == b;
}

/* = Polynomial = */
float
spm_poly_val_point(unsigned int poly_degree, float *poly_body, float point)
{
	float ret_val = 0.0;
	for (unsigned int i = 0; i < poly_degree; i++) {
		ret_val += poly_body[(poly_degree - 1) - i] * powf(point, i);
	}

	return ret_val;
}

#endif /* SPM_IMPLEMENTATION */
