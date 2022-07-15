#include <stdio.h>

#define MRSPC_HORNER_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/4-horner.h"

int
main(void)
{
	/* = Polynomial body = */
	unsigned int poly_degree;
	printf("Enter the degree of polynomial: ");
	scanf("%u", &poly_degree);

	float poly_body[poly_degree + 1];
	printf("Enter the coefficients: ");
	for (int i = 0; i <= poly_degree; i++) {
		scanf("%f", &poly_body[i]);
	}

	/* = Initialize horner instance = */
	struct hrn_t hrn_i;
	hrn_init(&hrn_i, poly_degree, poly_body);

	/* = Initial point = */
	float point;
	printf("Enter the point: ");
	scanf("%f", &point);

	/* = Process, Precision and Iterations = */
	enum hrn_process_t hrn_p;
	printf("What process do you want to execute?\n");
	printf("1. Till a number of iterations, 2. Correct upto n decimal places, 3. Correct upto n significant digits: ");
	scanf("%d", (int *)&hrn_p);

	int precision;
	printf("Precision? ");
	scanf("%d", &precision);

	int iterations_c;
	printf("How many iterations (at most) do you want to have? ");
	scanf("%d", &iterations_c);

	/* = Actual Work = */
	int                hrn_o_c;
	struct hrn_output *hrn_o = hrn_execute(&hrn_i, point, hrn_p, precision,
	                                       iterations_c, &hrn_o_c);

	precision += 3;
	/* = Print output = */
	for (int i = 0; i < hrn_o_c; i++) {
		printf("Iteration #%d:\n\n", i + 1);

		printf("%.*g\t|", precision + 1, hrn_o[i].x_input);
		for (int j = 0; j < hrn_i.poly_degree + 1; j++) {
			printf("%.*g\t", precision + 1, hrn_o[i].coeff_fn_1[j]);
		}
		printf("\n");
		printf("\t|\t");
		for (int j = 1; j < hrn_i.poly_degree + 1; j++) {
			printf("%.*g\t", precision + 1, hrn_o[i].coeff_fn_2[j]);
		}
		printf("\n");
		printf("\t----------------------------------------");
		printf("\n");
		printf("\t|");
		for (int j = 0; j < hrn_i.poly_degree + 1; j++) {
			printf("%.*g\t", precision + 1,
			       hrn_o[i].coeff_dfn_1[j]);
		}
		printf("\n");
		printf("\t|\t");
		for (int j = 1; j < hrn_i.poly_degree; j++) {
			printf("%.*g\t", precision + 1,
			       hrn_o[i].coeff_dfn_2[j]);
		}
		printf("\n");
		printf("\t----------------------------------------");
		printf("\n");
		printf("\t|");
		for (int j = 0; j < hrn_i.poly_degree; j++) {
			printf("%.*g\t", precision + 1, hrn_o[i].coeff_d2fn[j]);
		}
		printf("\n\n");
		printf("x%d = x%d - P%d(x%d)/P'%d(x%d) = %.*g - (%.*g)/(%.*g) = %.*g",
		       i + 1, i, hrn_i.poly_degree, i, hrn_i.poly_degree, i,
		       precision + 1, hrn_o[i].x_input, precision + 1,
		       *(hrn_o[i].p_x), precision + 1, *(hrn_o[i].dp_x),
		       precision + 1, hrn_o[i].x_output);
		printf("\n\n\n");
	}
}
