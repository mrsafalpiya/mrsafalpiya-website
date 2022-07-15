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
 *         #define MRSPC_SECANT_IMPLEMENTATION
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

#define MRSPC_SECANT_IMPLEMENTATION
#include "2-secant.h"

int
main(void)
{
	/* = Inputs for the bisection process = */
	char *input_expr     = "x^3 - 3*x + 1";       /* Input function */
	float interval_lower = 0, interval_upper = 1; /* Intervals */
	enum sct_process_t sct_p = decimal_places;    /* Process to execute */
	int precision = 3, iter_c = 99; /* Precision and Max iterations count */

	/* = Main process = */
	printf("Evaluating:\n\t%s\n", input_expr);
	struct sct_t sct_instance;
	int          expr_err_loc = sct_init(&sct_instance, input_expr);
	if (expr_err_loc != 0) {
		fprintf(stderr, "\t%*s^\nError near here\n", expr_err_loc - 1,
		        "");
		exit(EXIT_FAILURE);
	}

	int                sct_o_c;
	struct sct_output *sct_o =
		sct_execute(&sct_instance, interval_lower, interval_upper,
	                    sct_p, precision, iter_c, &sct_o_c);
	if (sct_o == NULL) {
		fprintf(stderr, "Invalid intervals\n");
		exit(EXIT_FAILURE);
	}

	/* = Display output = */
	for (int i = 0; i < sct_o_c; i++)
		printf("%d\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", i + 1,
		       precision + 1, sct_o[i].x0, precision + 1,
		       sct_o[i].fn_x0, precision + 1, sct_o[i].x1,
		       precision + 1, sct_o[i].fn_x1, precision + 1,
		       sct_o[i].x2, precision + 1, sct_o[i].fn_x2);

	/* = Cleanup and Exit = */
	sct_instance_free(&sct_instance);
	free(sct_o);
	return 0;
}
#endif

/* Output:
* Evaluating:
*         x^3 - 3*x + 1
* 1       0       1       1       -1      0.5     -0.375
* 2       1       -1      0.5     -0.375  0.2     0.408
* 3       0.5     -0.375  0.2     0.408   0.3563  -0.0237
* 4       0.2     0.408   0.3563  -0.0237 0.3477  -0.0011
* 5       0.3563  -0.0237 0.3477  -0.0011 0.3473  0
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef MRSPC_SECANT_H
#define MRSPC_SECANT_H

#include "../../../dep/tinyexpr.h"

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */
struct sct_t {
	te_expr *fn_expr;
	double   fn_x; /* Current (last) value that was used in the function. */
};

/* The process of getting root. */
enum sct_process_t {
	SCT_ITERATIONS = 1,
	SCT_DECIMAL_PLACES,
	SCT_SIGNIFICANT_DIGITS,
};

struct sct_output {
	float x0, fn_x0, x1, fn_x1, x2, fn_x2;
};

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
int
sct_init(struct sct_t *sct_instance, char *fn_expr_str);
/*
 * Initialize secant to use the given function expression.
 *
 * Fills up 'struct sct_t' which can be passed to other 'sct_*' functions for
 * further processing.
 *
 * Returns 0 if there was no problem with the expression or >0 specifying the
 * location where the problem was found.
 */

float
sct_point_val(struct sct_t *sct_instance, double point);
/* Calculate and return the value of the function at the given point. */

float
sct_next_x(struct sct_t *sct_instance, double x0, double x1);
/* Returns the next value of x from x0 and x1. */

struct sct_output *
sct_execute(struct sct_t *sct_instance, float interval_lower,
            float interval_upper, enum sct_process_t process,
            unsigned int precision, unsigned int iterations_c, int *n);
/*
 * Performs the actual secant process and returns the pointer to the array
 * containing the result.
 *
 * As the returned array is dynamically allocated, make sure to free each index
 * of the 'sct_output'.
 *
 * `*n` is filled with the number of iterations done in the process.
 *
 * Precision specifies the count for the specified `process`.
 *
 * At most `iterations_c` iterations are performed for all the `process`.
 */

void
sct_instance_free(struct sct_t *sct_instance);
/*
 * Destructor for the 'sct_t'.
 *
 * Actually the 'te_expr' inside the struct is free'ed.
 *
 * This is safe to call on NULL pointers.
 */

#endif /* MRSPC_SECANT_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef MRSPC_SECANT_IMPLEMENTATION

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
sct_init(struct sct_t *sct_instance, char *fn_expr_str)
{
	/* tinyexpr */
	te_variable fn_var[1] = { { "x", &(sct_instance->fn_x) } };

	int fn_expr_err;
	sct_instance->fn_expr =
		te_compile(fn_expr_str, fn_var, 1, &fn_expr_err);
	if (!sct_instance->fn_expr)
		return fn_expr_err;

	return 0;
}

float
sct_point_val(struct sct_t *sct_instance, double point)
{
	sct_instance->fn_x = point;

	return te_eval(sct_instance->fn_expr);
}

float
sct_next_x(struct sct_t *sct_instance, double x0, double x1)
{
	double fn_x0 = sct_point_val(sct_instance, x0);
	double fn_x1 = sct_point_val(sct_instance, x1);

	te_variable fn_vars[] = { { "x0", &x0 },
		                  { "x1", &x1 },
		                  { "fn_x0", &fn_x0 },
		                  { "fn_x1", &fn_x1 } };

	te_expr *fn_expr =
		te_compile("(x0 * fn_x1 - x1 * fn_x0) / (fn_x1 - fn_x0)",
	                   fn_vars, 4, NULL);

	float next_x = te_eval(fn_expr);

	te_free(fn_expr);
	return next_x;
}

struct sct_output *
sct_execute(struct sct_t *sct_instance, float interval_lower,
            float interval_upper, enum sct_process_t process,
            unsigned int precision, unsigned int iterations_c, int *n)
{
	float x0 = interval_lower;
	float x1 = interval_upper;

	int                count = 0;
	struct sct_output *sct_o_ret =
		malloc(iterations_c * sizeof(struct sct_output));
	for (unsigned int i = 0; i < iterations_c; i++) {
		float fn_x0 = sct_point_val(sct_instance, x0);
		float fn_x1 = sct_point_val(sct_instance, x1);
		float x2    = sct_next_x(sct_instance, x0, x1);
		float fn_x2 = sct_point_val(sct_instance, x2);

		if (process == SCT_ITERATIONS ||
		    process == SCT_DECIMAL_PLACES) {
			x0    = spm_round_off_d(x0, precision + 1);
			fn_x0 = spm_round_off_d(fn_x0, precision + 1);
			x1    = spm_round_off_d(x1, precision + 1);
			fn_x1 = spm_round_off_d(fn_x1, precision + 1);
			x2    = spm_round_off_d(x2, precision + 1);
			fn_x2 = spm_round_off_d(fn_x2, precision + 1);
		} else if (process == SCT_SIGNIFICANT_DIGITS) {
			x0    = spm_signifi_d(x0, precision + 1);
			fn_x0 = spm_signifi_d(fn_x0, precision + 1);
			x1    = spm_signifi_d(x1, precision + 1);
			fn_x1 = spm_signifi_d(fn_x1, precision + 1);
			x2    = spm_signifi_d(x2, precision + 1);
			fn_x2 = spm_signifi_d(fn_x2, precision + 1);
		}

		/* filling the output */
		sct_o_ret[i].x0    = x0;
		sct_o_ret[i].fn_x0 = fn_x0;
		sct_o_ret[i].x1    = x1;
		sct_o_ret[i].fn_x1 = fn_x1;
		sct_o_ret[i].x2    = x2;
		sct_o_ret[i].fn_x2 = fn_x2;

		count++;

		if (process == SCT_ITERATIONS ||
		    process == SCT_DECIMAL_PLACES) {
			if (spm_is_equal_deci(x1, x2, precision))
				break;
		} else {
			if (spm_is_equal_signi(x1, x2, precision))
				break;
		}

		x0    = x1;
		fn_x0 = fn_x1;
		x1    = x2;
		fn_x1 = fn_x2;
	}

	*n = count;
	return sct_o_ret;
}

void
sct_instance_free(struct sct_t *sct_instance)
{
	te_free(sct_instance->fn_expr);
}

#endif /* MRSPC_SECANT_IMPLEMENTATION */
