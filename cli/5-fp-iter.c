#include <stdio.h>
#include <stdlib.h>

#define MRSPC_FP_ITER_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/5-fixed-point-iteration.h"

int
main(void)
{
	/* = Equation = */
	char input_expr[128];
	printf("Enter the rearranged equation: ");
	fgets(input_expr, sizeof(input_expr) / sizeof(char), stdin);

	/* = Initialize fixed-point iteration instance = */
	printf("Evaluating:\n\t%s\n", input_expr);
	struct fp_iter_t fp_iter_instance;
	int expr_err_loc = fp_iter_init(&fp_iter_instance, input_expr);
	if (expr_err_loc != 0) {
		fprintf(stderr, "\t%*s^\nError near here\n", expr_err_loc - 1,
		        "");
		exit(EXIT_FAILURE);
	}

	/* = Point = */
	float point;
	printf("Enter the initial point: ");
	scanf("%f", &point);

	/* = Process, Precision and Iterations = */
	enum fp_iter_process_t fp_iter_p;
	printf("What process do you want to execute?\n");
	printf("1. Till a number of iterations, 2. Correct upto n decimal places, 3. Correct upto n significant digits: ");
	scanf("%d", (int *)&fp_iter_p);

	int precision;
	printf("Precision? ");
	scanf("%d", &precision);

	int iterations_c;
	printf("How many iterations (at most) do you want to have? ");
	scanf("%d", &iterations_c);

	/* = Actual Work = */
	struct fp_iter_output *fp_iter_o;
	int                    fp_iter_o_c;
	fp_iter_o = fp_iter_execute(&fp_iter_instance, point, fp_iter_p,
	                            precision, iterations_c, &fp_iter_o_c);

	/* = Print output = */
	for (int i = 0; i < fp_iter_o_c; i++)
		printf("%d\t%.*g\t%.*g\n", i + 1, precision + 2,
		       fp_iter_o[i].x_n, precision + 2, fp_iter_o[i].x_next);

	/* = Cleanup and Exit = */
	fp_iter_instance_free(&fp_iter_instance);
	/* TODO: Find out why this loop causes invalid free
	for (int i = 0; i < iterations_c; i++)
		free(fp_iter_o + i); */
	free(fp_iter_o);
	return 0;
}
