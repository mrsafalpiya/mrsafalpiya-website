#include <stdio.h>
#include <stdlib.h>

#define MRSPC_BISECTION_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/1-bisection.h"

int
main(void)
{
	/* = Equation = */
	char input_expr[128];
	printf("Enter the equation: ");
	fgets(input_expr, sizeof(input_expr) / sizeof(char), stdin);

	/* = Initialize bisection instance = */
	printf("Evaluating:\n\t%s\n", input_expr);
	struct bs_t bs_instance;
	int         expr_err_loc = bs_init(&bs_instance, input_expr);
	if (expr_err_loc != 0) {
		fprintf(stderr, "\t%*s^\nError near here\n", expr_err_loc - 1,
		        "");
		exit(EXIT_FAILURE);
	}

	/* = Intervals = */
	float interval_lower, interval_upper;
	printf("Enter the lower interval: ");
	scanf("%f", &interval_lower);
	printf("Enter the upper interval: ");
	scanf("%f", &interval_upper);

	/* = Process, Precision and Iterations = */
	enum bs_process_t bs_p;
	printf("What process do you want to execute?\n");
	printf("1. Till a number of iterations, 2. Correct upto n decimal places, 3. Correct upto n significant digits: ");
	scanf("%d", (int *)&bs_p);

	int precision;
	printf("Precision? ");
	scanf("%d", &precision);

	int iterations_c;
	printf("How many iterations (at most) do you want to have? ");
	scanf("%d", &iterations_c);

	/* = Actual Work = */
	int               bs_o_c;
	struct bs_output *bs_o = bs_execute(&bs_instance, interval_lower, interval_upper, bs_p,
	                  precision, iterations_c, &bs_o_c);

	/* = Print output = */
	for (int i = 0; i < bs_o_c; i++)
		printf("%d\t%.*g\t%c\t%.*g\t%c\t%.*g\t%c\n", i + 1,
		       precision + 2, bs_o[i].a, bs_o[i].fn_a_sign,
		       precision + 2, bs_o[i].b, bs_o[i].fn_b_sign,
		       precision + 2, bs_o[i].c, bs_o[i].fn_c_sign);

	/* = Cleanup and Exit = */
	bs_instance_free(&bs_instance);
	/* TODO: Find out why this loop causes invalid free
	for (int i = 0; i < iterations_c; i++)
		free(bs_o + i); */
	free(bs_o);
	return 0;
}
