#include <stdio.h>
#include <stdlib.h>

#define MRSPC_NEWTON_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/3-newton.h"

int
main(void)
{
	/* = Equation = */
	char input_expr[128];
	printf("Enter the equation: ");
	fgets(input_expr, sizeof(input_expr) / sizeof(char), stdin);

	char input_df_expr[128];
	printf("Enter the derivative equation: ");
	fgets(input_df_expr, sizeof(input_expr) / sizeof(char), stdin);

	/* = Initialize secant instance = */
	printf("Evaluating:\n\t%s\n", input_expr);
	struct nwtn_t nwtn_instance;
	int           expr_err_loc = nwtn_init(&nwtn_instance, input_expr);
	if (expr_err_loc != 0) {
		fprintf(stderr, "\t%*s^\nError near here\n", expr_err_loc - 1,
		        "");
		exit(EXIT_FAILURE);
	}

	printf("Evaluating:\n\t%s\n", input_df_expr);
	expr_err_loc = nwtn_init_df(&nwtn_instance, input_df_expr);
	if (expr_err_loc != 0) {
		fprintf(stderr, "\t%*s^\nError near here\n", expr_err_loc - 1,
		        "");
		exit(EXIT_FAILURE);
	}

	/* = Point = */
	float point;
	printf("Enter the point: ");
	scanf("%f", &point);

	/* = Process, Precision and Iterations = */
	enum nwtn_process_t nwtn_p;
	printf("What process do you want to execute?\n");
	printf("1. Till a number of iterations, 2. Correct upto n decimal places, 3. Correct upto n significant digits: ");
	scanf("%d", (int *)&nwtn_p);

	int precision;
	printf("Precision? ");
	scanf("%d", &precision);

	int iterations_c;
	printf("How many iterations (at most) do you want to have? ");
	scanf("%d", &iterations_c);

	/* = Actual Work = */
	struct nwtn_output *nwtn_o;
	int                 nwtn_o_c;
	nwtn_o = nwtn_execute(&nwtn_instance, point, nwtn_p, precision,
	                      iterations_c, &nwtn_o_c);

	/* = Print output = */
	for (int i = 0; i < nwtn_o_c; i++)
		printf("%d\t%.*g\t%.*g\t%.*g\t%.*g\n", i + 1, precision + 2,
		       nwtn_o[i].x0, precision + 2, nwtn_o[i].fn_x0,
		       precision + 1, nwtn_o[i].d_fn_x0, precision + 2,
		       nwtn_o[i].x1);

	/* = Cleanup and Exit = */
	nwtn_instance_free(&nwtn_instance);
	/* TODO: Find out why this loop causes invalid free
	for (int i = 0; i < iterations_c; i++)
		free(nwtn_o + i); */
	free(nwtn_o);
	return 0;
}
