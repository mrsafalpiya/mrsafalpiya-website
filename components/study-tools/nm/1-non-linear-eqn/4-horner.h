/*
 ===============================================================================
 |                                Dependencies                                 |
 ===============================================================================
 *
 * -> sp-math.h
 */

/*
 ===============================================================================
 |                                    Usage                                    |
 ===============================================================================
 *
 * Do this:
 *
 *         #define MRSPC_HORNER_IMPLEMENTATION
 *
 * before you include this file in *one* C or C++ file to create the
 * implementation.
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef MRSPC_HORNER_H
#define MRSPC_HORNER_H

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */
/* = Option = */
#define MRSPC_HORNER_MAX_DEGREE 16

struct hrn_t {
	unsigned int poly_degree;
	float       *poly_body;
};

/* The process of getting root. */
enum hrn_process_t {
	HRN_ITERATIONS = 1,
	HRN_DECIMAL_PLACES,
	HRN_SIGNIFICANT_DIGITS,
};

struct hrn_output {
	float x_input;

	float coeff_fn_1[MRSPC_HORNER_MAX_DEGREE];
	float coeff_fn_2[MRSPC_HORNER_MAX_DEGREE];
	float coeff_dfn_1[MRSPC_HORNER_MAX_DEGREE];
	float coeff_dfn_2[MRSPC_HORNER_MAX_DEGREE];
	float coeff_d2fn[MRSPC_HORNER_MAX_DEGREE];

	float *p_x;
	float *dp_x;

	float x_output;
};

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
void
hrn_init(struct hrn_t *hrn_instance, unsigned int poly_degree,
         float *poly_body);
/*
 * Initialize horner to use the given polynomial body.
 *
 * Fills up 'struct hrn_t' which can be passed to other 'hrn_*' functions for
 * further processing.
 */

struct hrn_output *
hrn_execute(struct hrn_t *hrn_instance, float point, enum hrn_process_t process,
            unsigned int precision, unsigned int iterations_c, int *n);
/*
 * Performs the actual horner process and returns the pointer to the array
 * containing the result.
 *
 * As the returned array is dynamically allocated, make sure to free each index
 * of the 'hrn_output'.
 *
 * `*n` is filled with the number of iterations done in the process.
 *
 * Precision specifies the count for the specified `process`.
 *
 * At most `iterations_c` iterations are performed for all the `process`.
 *
 * Returns NULL if the intervals aren't valid for the horner process.
 */

#endif /* MRSPC_HORNER_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef MRSPC_HORNER_IMPLEMENTATION

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
void
hrn_init(struct hrn_t *hrn_instance, unsigned int poly_degree, float *poly_body)
{
	hrn_instance->poly_degree = poly_degree;
	hrn_instance->poly_body   = poly_body;
}

struct hrn_output *
hrn_execute(struct hrn_t *hrn_instance, float point, enum hrn_process_t process,
            unsigned int precision, unsigned int iterations_c, int *n)
{
	int                count = 0;
	struct hrn_output *hrn_o_ret =
		malloc(iterations_c * sizeof(struct hrn_output));

	float point_old;
	for (unsigned int i = 0; i < iterations_c; i++) {
		hrn_o_ret[i].x_input = point;

		for (unsigned int j = 0; j < hrn_instance->poly_degree + 1; j++) {
			hrn_o_ret[i].coeff_fn_1[j] = hrn_instance->poly_body[j];
		}

		struct hrn_output *hrn_cur = &(hrn_o_ret[i]);
		float             *coeff_1, *coeff_2, *coeff_3;
		coeff_1    = &(hrn_cur->coeff_fn_1);
		coeff_2    = &(hrn_cur->coeff_fn_2);
		coeff_3    = &(hrn_cur->coeff_dfn_1);
		coeff_3[0] = coeff_1[0];
		for (int j = 1; j <= hrn_instance->poly_degree; j++) {
			coeff_2[j] = point * coeff_3[j - 1];
			coeff_3[j] = coeff_1[j] + coeff_2[j];
			if (process == HRN_ITERATIONS || process == HRN_DECIMAL_PLACES) {
				coeff_2[j] = spm_round_off_d(coeff_2[j], precision + 1);
				coeff_3[j] = spm_round_off_d(coeff_3[j], precision + 1);
			} else if (process == HRN_SIGNIFICANT_DIGITS) {
				coeff_2[j] = spm_signifi_d(coeff_2[j], precision + 1);
				coeff_3[j] = spm_signifi_d(coeff_3[j], precision + 1);
			}
		}
		hrn_cur->p_x = &(coeff_3[hrn_instance->poly_degree]);

		coeff_1    = &(hrn_cur->coeff_dfn_1);
		coeff_2    = &(hrn_cur->coeff_dfn_2);
		coeff_3    = &(hrn_cur->coeff_d2fn);
		coeff_3[0] = coeff_1[0];
		for (int j = 1; j <= hrn_instance->poly_degree - 1; j++) {
			coeff_2[j] = point * coeff_3[j - 1];
			coeff_3[j] = coeff_1[j] + coeff_2[j];
			if (process == HRN_ITERATIONS || process == HRN_DECIMAL_PLACES) {
				coeff_2[j] = spm_round_off_d(coeff_2[j], precision + 1);
				coeff_3[j] = spm_round_off_d(coeff_3[j], precision + 1);
			} else if (process == HRN_SIGNIFICANT_DIGITS) {
				coeff_2[j] = spm_signifi_d(coeff_2[j], precision + 1);
				coeff_3[j] = spm_signifi_d(coeff_3[j], precision + 1);
			}
		}
		hrn_cur->dp_x = &(coeff_3[hrn_instance->poly_degree - 1]);

		hrn_cur->x_output = point - *(hrn_cur->p_x) / *(hrn_cur->dp_x);
		if (process == HRN_ITERATIONS || process == HRN_DECIMAL_PLACES) {
			hrn_cur->x_output =
				spm_round_off_d(hrn_cur->x_output, precision + 1);
		} else if (process == HRN_SIGNIFICANT_DIGITS) {
			hrn_cur->x_output =
				spm_signifi_d(hrn_cur->x_output, precision + 1);
		}

		count++;

		/* check if we can stop */
		point_old = point;
		point = hrn_cur->x_output;
		if (process == HRN_ITERATIONS || process == HRN_DECIMAL_PLACES) {
			if (spm_is_equal_deci(point, point_old, precision))
				break;
		} else {
			if (spm_is_equal_signi(point, point_old, precision))
				break;
		}
	}

	*n = count;
	return hrn_o_ret;
}

#endif /* MRSPC_HORNER_IMPLEMENTATION */
