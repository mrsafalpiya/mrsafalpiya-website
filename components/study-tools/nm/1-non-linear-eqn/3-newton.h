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
 *         #define MRSPC_NEWTON_IMPLEMENTATION
 *
 * before you include this file in *one* C or C++ file to create the
 * implementation.
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef MRSPC_NEWTON_H
#define MRSPC_NEWTON_H

#include "../../../dep/tinyexpr.h"

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */
struct nwtn_t {
	te_expr *fn_expr;
	te_expr *d_fn_expr;
	double   fn_x; /* Current (last) value that was used in the function. */
};

/* The process of getting root. */
enum nwtn_process_t {
	nwtn_ITERATIONS = 1,
	nwtn_DECIMAL_PLACES,
	nwtn_SIGNIFICANT_DIGITS,
};

struct nwtn_output {
	float x0, fn_x0, d_fn_x0, x1;
};

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
int
nwtn_init(struct nwtn_t *nwtn_instance, char *fn_expr_str);
/*
 * Initialize newton to use the given function expression.
 *
 * NOTE: Make sure to run the 'nwtn_init_df()' after this function.
 *
 * Fills up 'struct nwtn_t' which can be passed to other 'nwtn_*' functions for
 * further processing.
 *
 * Returns 0 if there was no problem with the expression or >0 specifying the
 * location where the problem was found.
 */

int
nwtn_init_df(struct nwtn_t *nwtn_instance, char *d_fn_expr_str);
/*
 * Initialize newton to use the given derivative function expression.
 *
 * Fills up 'struct nwtn_t' which can be passed to other 'nwtn_*' functions for
 * further processing.
 *
 * Returns 0 if there was no problem with the expression or >0 specifying the
 * location where the problem was found.
 */

float
nwtn_point_val(struct nwtn_t *nwtn_instance, double point);
/* Calculate and return the value of the function at the given point. */

float
nwtn_df_point_val(struct nwtn_t *nwtn_instance, double point);
/* Calculate and return the value of the derivative function at the given point. */

float
nwtn_next_x(struct nwtn_t *nwtn_instance, double x0, double fn_x0,
            double d_fn_x0);
/* Returns the next value of x from x0, fn_x0 and fn_x1. */

struct nwtn_output *
nwtn_execute(struct nwtn_t *nwtn_instance, float point,
             enum nwtn_process_t process, unsigned int precision,
             unsigned int iterations_c, int *n);
/*
 * Performs the actual newton process and returns the pointer to the array
 * containing the result.
 *
 * As the returned array is dynamically allocated, make sure to free each index
 * of the 'nwtn_output'.
 *
 * `*n` is filled with the number of iterations done in the process.
 *
 * Precision specifies the count for the specified `process`.
 *
 * At most `iterations_c` iterations are performed for all the `process`.
 */

void
nwtn_instance_free(struct nwtn_t *nwtn_instance);
/*
 * Destructor for the 'nwtn_t'.
 *
 * Actually the 'te_expr' inside the struct is free'ed.
 *
 * This is safe to call on NULL pointers.
 */

#endif /* MRSPC_NEWTON_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef MRSPC_NEWTON_IMPLEMENTATION

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
nwtn_init(struct nwtn_t *nwtn_instance, char *fn_expr_str)
{
	/* tinyexpr */
	te_variable fn_var[1] = { { "x", &(nwtn_instance->fn_x) } };

	int fn_expr_err;
	nwtn_instance->fn_expr =
		te_compile(fn_expr_str, fn_var, 1, &fn_expr_err);
	if (!nwtn_instance->fn_expr)
		return fn_expr_err;

	return 0;
}

int
nwtn_init_df(struct nwtn_t *nwtn_instance, char *d_fn_expr_str)
{
	/* tinyexpr */
	te_variable fn_var[1] = { { "x", &(nwtn_instance->fn_x) } };

	int fn_expr_err;
	nwtn_instance->d_fn_expr =
		te_compile(d_fn_expr_str, fn_var, 1, &fn_expr_err);
	if (!nwtn_instance->fn_expr)
		return fn_expr_err;

	return 0;
}

float
nwtn_point_val(struct nwtn_t *nwtn_instance, double point)
{
	nwtn_instance->fn_x = point;

	return te_eval(nwtn_instance->fn_expr);
}

float
nwtn_df_point_val(struct nwtn_t *nwtn_instance, double point)
{
	nwtn_instance->fn_x = point;

	return te_eval(nwtn_instance->d_fn_expr);
}

float
nwtn_next_x(struct nwtn_t *nwtn_instance, double x0, double fn_x0,
            double d_fn_x0)
{
	te_variable fn_vars[] = { { "x0", &x0 },
		                  { "fn_x0", &fn_x0 },
		                  { "d_fn_x0", &d_fn_x0 } };

	te_expr *fn_expr =
		te_compile("x0 - (fn_x0 / d_fn_x0)", fn_vars, 3, NULL);

	float next_x = te_eval(fn_expr);

	te_free(fn_expr);
	return next_x;
}

struct nwtn_output *
nwtn_execute(struct nwtn_t *nwtn_instance, float point,
             enum nwtn_process_t process, unsigned int precision,
             unsigned int iterations_c, int *n)
{
	int                 count = 0;
	struct nwtn_output *nwtn_o_ret =
		malloc(iterations_c * sizeof(struct nwtn_output));

	float old_x1;
	for (unsigned int i = 0; i < iterations_c; i++) {
		float fn_x0   = nwtn_point_val(nwtn_instance, point);
		float d_fn_x0 = nwtn_df_point_val(nwtn_instance, point);
		float x1 = nwtn_next_x(nwtn_instance, point, fn_x0, d_fn_x0);

		if (process == nwtn_ITERATIONS ||
		    process == nwtn_DECIMAL_PLACES) {
			fn_x0   = spm_round_off_d(fn_x0, precision + 1);
			d_fn_x0 = spm_round_off_d(d_fn_x0, precision + 1);
			x1      = spm_round_off_d(x1, precision + 1);
		} else if (process == nwtn_SIGNIFICANT_DIGITS) {
			fn_x0   = spm_signifi_d(fn_x0, precision + 1);
			d_fn_x0 = spm_signifi_d(d_fn_x0, precision + 1);
			x1      = spm_signifi_d(x1, precision + 1);
		}

		/* filling the output */
		nwtn_o_ret[i].x0      = point;
		nwtn_o_ret[i].fn_x0   = fn_x0;
		nwtn_o_ret[i].d_fn_x0 = d_fn_x0;
		nwtn_o_ret[i].x1      = x1;

		count++;

		/* check if we can stop */
		old_x1 = point;
		point     = x1;

		if (process == nwtn_ITERATIONS ||
		    process == nwtn_DECIMAL_PLACES) {
			if (spm_is_equal_deci(point, old_x1, precision))
				break;
		} else {
			if (spm_is_equal_signi(point, old_x1, precision))
				break;
		}
	}

	*n = count;
	return nwtn_o_ret;
}

void
nwtn_instance_free(struct nwtn_t *nwtn_instance)
{
	te_free(nwtn_instance->fn_expr);
	te_free(nwtn_instance->d_fn_expr);
}

#endif /* MRSPC_NEWTON_IMPLEMENTATION */
