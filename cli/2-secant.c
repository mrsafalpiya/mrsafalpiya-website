#include <stdio.h>
#include <stdlib.h>

#define MRSPC_SECANT_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/2-secant.h"

int
main(void)
{
	/* = Equation = */
	char input_expr[128];
	printf("Enter the equation: ");
	fgets(input_expr, sizeof(input_expr) / sizeof(char), stdin);

	/* = Initialize secant instance = */
	printf("Evaluating:\n\t%s\n", input_expr);
	struct sct_t sct_instance;
	int          expr_err_loc = sct_init(&sct_instance, input_expr);
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
	enum sct_process_t sct_p;
	printf("What process do you want to execute?\n");
	printf("1. Till a number of iterations, 2. Correct upto n decimal places, 3. Correct upto n significant digits: ");
	scanf("%d", (int *)&sct_p);

	int precision;
	printf("Precision? ");
	scanf("%d", &precision);

	int iterations_c;
	printf("How many iterations (at most) do you want to have? ");
	scanf("%d", &iterations_c);

	/* = Actual Work = */
	struct sct_output *sct_o;
	int                sct_o_c;
	sct_o = sct_execute(&sct_instance, interval_lower, interval_upper,
	                    sct_p, precision, iterations_c, &sct_o_c);

	/* = Print output = */
	for (int i = 0; i < sct_o_c; i++)
		printf("%d\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", i + 1,
		       precision + 2, sct_o[i].x0, precision + 2,
		       sct_o[i].fn_x0, precision + 1, sct_o[i].x1,
		       precision + 2, sct_o[i].fn_x1, precision + 2,
		       sct_o[i].x2, precision + 2, sct_o[i].fn_x2);

	/* = Cleanup and Exit = */
	sct_instance_free(&sct_instance);
	/* TODO: Find out why this loop causes invalid free
	for (int i = 0; i < iterations_c; i++)
		free(sct_o + i); */
	free(sct_o);
	return 0;
}
