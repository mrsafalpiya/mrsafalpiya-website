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
 *         #define MRSPC_FP_ITER_IMPLEMENTATION
 *
 * before you include this file in *one* C or C++ file to create the
 * implementation.
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef MRSPC_FP_ITER_H
#define MRSPC_FP_ITER_H

#include "../../../dep/tinyexpr.h"

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */
struct fp_iter_t {
	te_expr *fn_expr;
	double   fn_x; /* Current (last) value that was used in the function. */
};

/* The process of getting root. */
enum fp_iter_process_t {
	fp_iter_ITERATIONS = 1,
	fp_iter_DECIMAL_PLACES,
	fp_iter_SIGNIFICANT_DIGITS,
};

struct fp_iter_output {
	float x_n, x_next;
};

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
int
fp_iter_init(struct fp_iter_t *fp_iter_instance, char *fn_expr_str);
/*
 * Initialize fixed-point iteration to use the given function expression.
 *
 * Fills up 'struct fp_iter_t' which can be passed to other 'fp_iter_*' functions for
 * further processing.
 *
 * Returns 0 if there was no problem with the expression or >0 specifying the
 * location where the problem was found.
 */

float
fp_iter_point_val(struct fp_iter_t *fp_iter_instance, double point);
/* Calculate and return the value of the function at the given point. */

struct fp_iter_output *
fp_iter_execute(struct fp_iter_t *fp_iter_instance, float point,
                enum fp_iter_process_t process, unsigned int precision,
                unsigned int iterations_c, int *n);
/*
 * Performs the actual fixed-point iteration process and returns the pointer to the array
 * containing the result.
 *
 * As the returned array is dynamically allocated, make sure to free each index
 * of the 'fp_iter_output'.
 *
 * `*n` is filled with the number of iterations done in the process.
 *
 * Precision specifies the count for the specified `process`.
 *
 * At most `iterations_c` iterations are performed for all the `process`.
 */

void
fp_iter_instance_free(struct fp_iter_t *fp_iter_instance);
/*
 * Destructor for the 'fp_iter_t'.
 *
 * Actually the 'te_expr' inside the struct is free'ed.
 *
 * This is safe to call on NULL pointers.
 */

#endif /* MRSPC_FP_ITER_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef MRSPC_FP_ITER_IMPLEMENTATION

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
fp_iter_init(struct fp_iter_t *fp_iter_instance, char *fn_expr_str)
{
	/* tinyexpr */
	te_variable fn_var[1] = { { "x", &(fp_iter_instance->fn_x) } };

	int fn_expr_err;
	fp_iter_instance->fn_expr =
		te_compile(fn_expr_str, fn_var, 1, &fn_expr_err);
	if (!fp_iter_instance->fn_expr)
		return fn_expr_err;

	return 0;
}

float
fp_iter_point_val(struct fp_iter_t *fp_iter_instance, double point)
{
	fp_iter_instance->fn_x = point;

	return te_eval(fp_iter_instance->fn_expr);
}

struct fp_iter_output *
fp_iter_execute(struct fp_iter_t *fp_iter_instance, float point,
                enum fp_iter_process_t process, unsigned int precision,
                unsigned int iterations_c, int *n)
{
	int                    count = 0;
	struct fp_iter_output *fp_iter_o_ret =
		malloc(iterations_c * sizeof(struct fp_iter_output));
	for (unsigned int i = 0; i < iterations_c; i++) {
		float next_point = fp_iter_point_val(fp_iter_instance, point);

		if (process == fp_iter_ITERATIONS ||
		    process == fp_iter_DECIMAL_PLACES) {
			next_point = spm_round_off_d(next_point, precision + 1);
		} else if (process == fp_iter_SIGNIFICANT_DIGITS) {
			next_point = spm_signifi_d(next_point, precision + 1);
		}

		/* filling the output */
		fp_iter_o_ret[i].x_n = point;
		fp_iter_o_ret[i].x_next = next_point;

		count++;

		/* check if we can stop */
		if (process == fp_iter_ITERATIONS ||
		    process == fp_iter_DECIMAL_PLACES) {
			if (spm_is_equal_deci(point, next_point, precision))
				break;
		} else {
			if (spm_is_equal_signi(point, next_point, precision))
				break;
		}

		/* prepare for next iteration */
		point = next_point;
	}

	*n = count;
	return fp_iter_o_ret;
}

void
fp_iter_instance_free(struct fp_iter_t *fp_iter_instance)
{
	te_free(fp_iter_instance->fn_expr);
}

#endif /* MRSPC_FP_ITER_IMPLEMENTATION */
