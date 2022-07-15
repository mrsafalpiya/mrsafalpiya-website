/*
 ===============================================================================
 |                                Dependencies                                 |
 ===============================================================================
 *
 * -> tinyexpr
 * -> sp-math.h
 */

/*
 ===============================================================================
 |                                    Usage                                    |
 ===============================================================================
 *
 * Do this:
 *
 *         #define MRSPC_BISECTION_IMPLEMENTATION
 *
 * before you include this file in *one* C or C++ file to create the
 * implementation.
 */

/*
 ===============================================================================
 |                                Example code                                 |
 ===============================================================================
 */
#if 0
#include <stdio.h>

#define MRSPC_BISECTION_IMPLEMENTATION
#include "1-bisection.h"

int
main(void)
{
	/* = Inputs for the bisection process = */
	char *input_expr     = "x^3 - 2 * sin x";       /* Input function */
	float interval_lower = 0.5, interval_upper = 2; /* Intervals */
	enum bs_process_t bs_p = significant_digits;    /* Process to execute */
	int precision = 5, iter_c = 99; /* Precision and Max iterations count */

	/* = Main process = */
	printf("Evaluating:\n\t%s\n", input_expr);
	struct bs_t bs_instance;
	int         expr_err_loc = bs_init(&bs_instance, input_expr);
	if (expr_err_loc != 0) {
		fprintf(stderr, "\t%*s^\nError near here\n", expr_err_loc - 1,
		        "");
		exit(EXIT_FAILURE);
	}

	int               bs_o_c;
	struct bs_output *bs_o =
		bs_execute(&bs_instance, interval_lower, interval_upper, bs_p,
	                   precision, iter_c, &bs_o_c);
	if (bs_o == NULL) {
		fprintf(stderr, "Invalid intervals\n");
		exit(EXIT_FAILURE);
	}

	/* = Display output = */
	for (int i = 0; i < bs_o_c; i++)
		printf("%d\t%.*g\t%c\t%.*g\t%c\t%.*g\t%c\n", i + 1,
		       precision + 1, bs_o[i].a, bs_o[i].fn_a_sign,
		       precision + 1, bs_o[i].b, bs_o[i].fn_b_sign,
		       precision + 1, bs_o[i].c, bs_o[i].fn_c_sign);

	/* = Cleanup and Exit = */
	bs_instance_free(&bs_instance);
	free(bs_o);
	return 0;
}
#endif
/* Output:
 * Evaluating:
 *         x^3 - 2 * sin x
 * 1       0.5     -       2       +       1.25    +
 * 2       0.5     -       1.25    +       0.875   -
 * 3       0.875   -       1.25    +       1.0625  -
 * 4       1.0625  -       1.25    +       1.15625 -
 * 5       1.15625 -       1.25    +       1.20312 -
 * 6       1.20312 -       1.25    +       1.22656 -
 * 7       1.22656 -       1.25    +       1.23828 +
 * 8       1.22656 -       1.23828 +       1.23242 -
 * 9       1.23242 -       1.23828 +       1.23535 -
 * 10      1.23535 -       1.23828 +       1.23682 +
 * 11      1.23535 -       1.23682 +       1.23608 -
 * 12      1.23608 -       1.23682 +       1.23645 +
 * 13      1.23608 -       1.23645 +       1.23626 +
 * 14      1.23608 -       1.23626 +       1.23617 -
 * 15      1.23617 -       1.23626 +       1.23622 +
 * 16      1.23617 -       1.23622 +       1.2362  +
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef MRSPC_BISECTION_H
#define MRSPC_BISECTION_H

#include "../../../dep/tinyexpr.h"

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */
struct bs_t {
	te_expr *fn_expr;
	double   fn_x; /* Current (last) value that was used in the function. */
};

/* The process of getting root. */
enum bs_process_t {
	BS_ITERATIONS = 1,
	BS_DECIMAL_PLACES,
	BS_SIGNIFICANT_DIGITS,
};

struct bs_output {
	float a, b, c;
	char  fn_a_sign, fn_b_sign, fn_c_sign;
};

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
int
bs_init(struct bs_t *bs_instance, char *fn_expr_str);
/*
 * Initialize bisection to use the given function expression.
 *
 * Fills up 'struct bs_t' which can be passed to other 'bs_*' functions for
 * further processing.
 *
 * Returns 0 if there was no problem with the expression or >0 specifying the
 * location where the problem was found.
 */

float
bs_point_val(struct bs_t *bs_instance, double point);
/* Calculate and return the value of the function at the given point. */

char
bs_point_val_sign(struct bs_t *bs_instance, double point);
/*
 * Get the sign of the value of function at the given point.
 *
 * Returns '+' for positive and '-' for negative.
 */

int
bs_are_valid_points(struct bs_t *bs_instance, double point1, double point2);
/* Check if the two points are a valid points for bisection method. */

struct bs_output *
bs_execute(struct bs_t *bs_instance, float interval_lower, float interval_upper,
           enum bs_process_t process, unsigned int precision,
           unsigned int iterations_c, int *n);
/*
 * Performs the actual bisection process and returns the pointer to the array
 * containing the result.
 *
 * As the returned array is dynamically allocated, make sure to free each index
 * of the 'bs_output'.
 *
 * `*n` is filled with the number of iterations done in the process.
 *
 * Precision specifies the count for the specified `process`.
 *
 * At most `iterations_c` iterations are performed for all the `process`.
 *
 * Returns NULL if the intervals aren't valid for the bisection process.
 */

void
bs_instance_free(struct bs_t *bs_instance);
/*
 * Destructor for the 'bs_t'.
 *
 * Actually the 'te_expr' inside the struct is free'ed.
 *
 * This is safe to call on NULL pointers.
 */

#endif /* MRSPC_BISECTION_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef MRSPC_BISECTION_IMPLEMENTATION

#include <stdlib.h>

#ifndef SPM_IMPLEMENTED /* Avoid sp-math.h's implementation twice. */
#define SPM_IMPLEMENTATION
#include "../../../dep/sp-math.h"
#endif

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */
int
bs_init(struct bs_t *bs_instance, char *fn_expr_str)
{
	/* tinyexpr */
	te_variable fn_var[1] = { { "x", &(bs_instance->fn_x) } };

	int fn_expr_err;
	bs_instance->fn_expr = te_compile(fn_expr_str, fn_var, 1, &fn_expr_err);
	if (!bs_instance->fn_expr)
		return fn_expr_err;

	return 0;
}

float
bs_point_val(struct bs_t *bs_instance, double point)
{
	bs_instance->fn_x = point;

	return te_eval(bs_instance->fn_expr);
}

char
bs_point_val_sign(struct bs_t *bs_instance, double point)
{
	if (bs_point_val(bs_instance, point) > 0)
		return '+';

	return '-';
}

int
bs_are_valid_points(struct bs_t *bs_instance, double point1, double point2)
{
	double val1 = bs_point_val(bs_instance, point1);
	double val2 = bs_point_val(bs_instance, point2);

	if ((val1 < 0 && val2 < 0) || (val1 > 0 && val2 > 0))
		return 0;

	return 1;
}

struct bs_output *
bs_execute(struct bs_t *bs_instance, float interval_lower, float interval_upper,
           enum bs_process_t process, unsigned int precision,
           unsigned int iterations_c, int *n)
{
	if (!bs_are_valid_points(bs_instance, interval_lower, interval_upper))
		return NULL;

	float a = interval_lower;
	float b = interval_upper;

	int               count = 0;
	struct bs_output *bs_o_ret =
		malloc(iterations_c * sizeof(struct bs_output));

	/* Initialize '*c_old' to something so that it won't seg fault in the
	 * first 'is_equal_*' comparision. */
	float *c_old = &b;
	for (unsigned int i = 0; i < iterations_c; i++) {
		char  fn_a_sign = bs_point_val_sign(bs_instance, a);
		char  fn_b_sign = bs_point_val_sign(bs_instance, b);
		float c         = (a + b) / 2.0f;
		char  fn_c_sign = bs_point_val_sign(bs_instance, c);

		if (process == BS_ITERATIONS || process == BS_DECIMAL_PLACES) {
			a = spm_round_off_d(a, precision + 1);
			b = spm_round_off_d(b, precision + 1);
			c = spm_round_off_d(c, precision + 1);
		} else if (process == BS_SIGNIFICANT_DIGITS) {
			a = spm_signifi_d(a, precision + 1);
			b = spm_signifi_d(b, precision + 1);
			c = spm_signifi_d(c, precision + 1);
		}

		/* filling the output */
		bs_o_ret[i].a         = a;
		bs_o_ret[i].fn_a_sign = fn_a_sign;
		bs_o_ret[i].b         = b;
		bs_o_ret[i].fn_b_sign = fn_b_sign;
		bs_o_ret[i].c         = c;
		bs_o_ret[i].fn_c_sign = fn_c_sign;

		count++;

		if (process == BS_ITERATIONS || process == BS_DECIMAL_PLACES) {
			if (spm_is_equal_deci(c, *c_old, precision))
				break;
		} else {
			if (spm_is_equal_signi(c, *c_old, precision))
				break;
		}

		if ((fn_c_sign == '+' && fn_a_sign == '+') ||
		    (fn_c_sign == '-' && fn_a_sign == '-')) {
			a     = c;
			c_old = &a;
		} else {
			b     = c;
			c_old = &b;
		}
	}

	*n = count;
	return bs_o_ret;
}

void
bs_instance_free(struct bs_t *bs_instance)
{
	te_free(bs_instance->fn_expr);
}

#endif /* MRSPC_BISECTION_IMPLEMENTATION */
