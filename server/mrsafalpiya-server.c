#include <stdio.h>
#include <string.h>

/* mongoose - https://github.com/cesanta/mongoose */
#include "dep/mongoose.h"

/* json - http://ccodearchive.net/info/json.html */
#include "dep/json.h"

/* spl - https://github.com/mrsafalpiya/spl */
#define SPLU_IMPLEMENTATION
#include "dep/spl_utils.h"

#include "dep/spl_flags.h"

/* components */
#define MRSPC_BISECTION_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/1-bisection.h"
#define MRSPC_SECANT_IMPLEMENTATION
#include "../components/study-tools/nm/1-non-linear-eqn/2-secant.h"

/* config file */
#include "config.h"

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */
/* = Server = */
char *executable_path;
#define JSON_STR_MAX 8192

/* = Interrupts = */
static int s_signo;

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */
/* = Core = */
static void
s_handler_fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
/* Main server handler. */

static int
s_hm_get_data(struct mg_connection *c, JsonNode *hm_body, char *key, char *info,
              unsigned int type, unsigned int is_required, void *data);
/*
 * Search and get the `key` from the `hm_body` into `data`.
 *
 * `type` includes: 0 for string, 1 for float, 2 for double, 3 for int, 4 for
 * bool.
 *
 * If the key is required but isn't found, a 400 response will be given asking
 * to provide the key written as the `info`.
 */

void
print_help_exit(FILE *stream, int exit_code);
/*
 * Print help to the given `stream` and exit the program with the given
 * `exit_code`.
 */

static void
signal_handler(int signo);

/* = Server components = */
/*
 * Naming convention: s_handler_c_<topic>_<section>_<subsection>_<name>
 * where <topic> can be 'st': study-tools.
 */
static void
s_handler_c_st_nm_1_bisection(struct mg_connection   *c,
                              struct mg_http_message *hm);

static void
s_handler_c_st_nm_1_secant(struct mg_connection *c, struct mg_http_message *hm);

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */
/* = Core = */
static void
s_handler_fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
	struct mg_http_message *hm;

	(void)fn_data;

	if (ev != MG_EV_HTTP_MSG)
		return;

	hm = (struct mg_http_message *)ev_data;
	/* = Study Tools = */
	if (mg_http_match_uri(hm, URI_STUDY_TOOLS
	                      "/nm/1-non-linear-eqn/1-bisection")) {
		if (strncmp(hm->method.ptr, "POST", 4) == 0)
			s_handler_c_st_nm_1_bisection(c, hm);

		mg_http_reply(c, 400, "",
		              "This uri supports only POST method.");

		return;
	}
	if (mg_http_match_uri(hm, URI_STUDY_TOOLS
	                      "/nm/1-non-linear-eqn/2-secant")) {
		if (strncmp(hm->method.ptr, "POST", 4) == 0)
			s_handler_c_st_nm_1_secant(c, hm);

		mg_http_reply(c, 400, "",
		              "This uri supports only POST method.");

		return;
	}
}

static int
s_hm_get_data(struct mg_connection *c, JsonNode *hm_body, char *key, char *info,
              unsigned int type, unsigned int is_required, void *data)
{
	JsonNode *hm_body_member = json_find_member(hm_body, key);
	if (!hm_body_member && is_required) {
		mg_http_reply(c, 400, "", "Please provide the %s.", info);
		return 0;
	}

	switch (type) {
	case 0:
		strcpy((char *)data, hm_body_member->string_);
		break;
	case 1:
		*((float *)data) = hm_body_member->number_;
		break;
	case 2:
		*((double *)data) = hm_body_member->number_;
		break;
	case 3:
		*((int *)data) = hm_body_member->number_;
		break;
	case 4:
		*((bool *)data) = hm_body_member->bool_;
		break;
	default:
		break;
	}

	return 1;
}

void
print_help_exit(FILE *stream, int exit_code)
{
	fprintf(stream, "Usage: %s [options]\n", executable_path);
	spl_flags_print_help(stream);

	exit(exit_code);
}

static void
signal_handler(int signo)
{
	s_signo = signo;
}

/* = Server components = */
static void
s_handler_c_st_nm_1_bisection(struct mg_connection   *c,
                              struct mg_http_message *hm)
{
	/* = Read the inputs = */
	JsonNode *hm_body = json_decode(hm->body.ptr);
	/* input_expr */
	char input_expr[512];
	if (!s_hm_get_data(c, hm_body, "input_expr", "input expression", 0, 1,
	                   &input_expr))
		return;
	/* intervals */
	float interval_lower = 3;
	if (!s_hm_get_data(c, hm_body, "interval_lower", "lower interval", 1, 1,
	                   &interval_lower))
		return;
	float interval_upper;
	if (!s_hm_get_data(c, hm_body, "interval_upper", "upper interval", 1, 1,
	                   &interval_upper))
		return;
	/* process */
	enum bs_process_t bs_p;
	if (!s_hm_get_data(c, hm_body, "bs_p", "bisection process", 3, 1,
	                   &bs_p))
		return;
	/* precision */
	int precision;
	if (!s_hm_get_data(c, hm_body, "precision", "precision", 3, 1,
	                   &precision))
		return;
	/* iterations */
	int iterations;
	if (!s_hm_get_data(c, hm_body, "iterations", "iterations", 3, 1,
	                   &iterations))
		return;
	/* cleanup */
	json_delete(hm_body);

	/* = Main process = */
	struct bs_t bs_instance;
	int         expr_err_loc = bs_init(&bs_instance, input_expr);
	if (expr_err_loc != 0) {
		/* error in the expression */
		JsonNode *position_error_json = json_mkobject();
		json_append_member(position_error_json, "message",
		                   json_mkstring("Error in the expression"));
		json_append_member(position_error_json, "position",
		                   json_mknumber(expr_err_loc));
		char *position_error_json_str =
			json_stringify(position_error_json, "\t");

		mg_http_reply(c, 400, "Content-Type: application/json\r\n",
		              position_error_json_str);

		json_delete(position_error_json);
		free(position_error_json_str);
		return;
	}

	int               bs_o_c;
	struct bs_output *bs_o =
		bs_execute(&bs_instance, interval_lower, interval_upper, bs_p,
	                   precision, iterations, &bs_o_c);
	if (bs_o == NULL) {
		fprintf(stderr, "Invalid intervals\n");
		exit(EXIT_FAILURE);
	}

	/* = Prepare output = */
	/* create JSON for the output */
	JsonNode *bs_o_json = json_mkarray();

	/* fill json string */
	for (int i = 0; i < bs_o_c; i++) {
		JsonNode *bs_item_json = json_mkobject();

		/* prepare object */
		char sign[2] = "\0";
		json_append_member(bs_item_json, "n", json_mknumber(i + 1));
		json_append_member(bs_item_json, "a", json_mknumber(bs_o[i].a));
		sign[0] = bs_o[i].fn_a_sign;
		json_append_member(bs_item_json, "fn_a", json_mkstring(sign));
		json_append_member(bs_item_json, "b", json_mknumber(bs_o[i].b));
		sign[0] = bs_o[i].fn_b_sign;
		json_append_member(bs_item_json, "fn_b", json_mkstring(sign));
		json_append_member(bs_item_json, "c", json_mknumber(bs_o[i].c));
		sign[0] = bs_o[i].fn_c_sign;
		json_append_member(bs_item_json, "fn_c", json_mkstring(sign));

		/* append the object to the array */
		json_append_element(bs_o_json, bs_item_json);
	}
	char *bs_o_json_str = json_stringify(bs_o_json, "\t");

	/* Reply with the JSON */
	mg_http_reply(c, 200, "Content-Type: application/json\r\n",
	              bs_o_json_str);

	/* = Cleanup = */
	json_delete(bs_o_json);
	free(bs_o_json_str);
	bs_instance_free(&bs_instance);
	free(bs_o);
}

static void
s_handler_c_st_nm_1_secant(struct mg_connection *c, struct mg_http_message *hm)
{
	/* = Read the inputs = */
	JsonNode *hm_body = json_decode(hm->body.ptr);
	/* input_expr */
	char input_expr[512];
	if (!s_hm_get_data(c, hm_body, "input_expr", "input expression", 0, 1,
	                   &input_expr))
		return;
	/* intervals */
	float interval_lower = 3;
	if (!s_hm_get_data(c, hm_body, "interval_lower", "lower interval", 1, 1,
	                   &interval_lower))
		return;
	float interval_upper;
	if (!s_hm_get_data(c, hm_body, "interval_upper", "upper interval", 1, 1,
	                   &interval_upper))
		return;
	/* process */
	enum sct_process_t sct_p;
	if (!s_hm_get_data(c, hm_body, "sct_p", "secant process", 3, 1,
	                   &sct_p))
		return;
	/* precision */
	int precision;
	if (!s_hm_get_data(c, hm_body, "precision", "precision", 3, 1,
	                   &precision))
		return;
	/* iterations */
	int iterations;
	if (!s_hm_get_data(c, hm_body, "iterations", "iterations", 3, 1,
	                   &iterations))
		return;
	/* cleanup */
	json_delete(hm_body);

	/* = Main process = */
	struct sct_t sct_instance;
	int         expr_err_loc = sct_init(&sct_instance, input_expr);
	if (expr_err_loc != 0) {
		/* error in the expression */
		JsonNode *position_error_json = json_mkobject();
		json_append_member(position_error_json, "message",
		                   json_mkstring("Error in the expression"));
		json_append_member(position_error_json, "position",
		                   json_mknumber(expr_err_loc));
		char *position_error_json_str =
			json_stringify(position_error_json, "\t");

		mg_http_reply(c, 400, "Content-Type: application/json\r\n",
		              position_error_json_str);

		json_delete(position_error_json);
		free(position_error_json_str);
		return;
	}

	int               sct_o_c;
	struct sct_output *sct_o =
		sct_execute(&sct_instance, interval_lower, interval_upper, sct_p,
	                   precision, iterations, &sct_o_c);

	/* = Prepare output = */
	/* create JSON for the output */
	JsonNode *sct_o_json = json_mkarray();

	/* fill json string */
	for (int i = 0; i < sct_o_c; i++) {
		JsonNode *sct_item_json = json_mkobject();

		/* prepare object */
		json_append_member(sct_item_json, "n", json_mknumber(i + 1));
		json_append_member(sct_item_json, "x0", json_mknumber(sct_o[i].x0));
		json_append_member(sct_item_json, "fn_x0", json_mknumber(sct_o[i].fn_x0));
		json_append_member(sct_item_json, "x1", json_mknumber(sct_o[i].x1));
		json_append_member(sct_item_json, "fn_x1", json_mknumber(sct_o[i].fn_x1));
		json_append_member(sct_item_json, "x2", json_mknumber(sct_o[i].x2));
		json_append_member(sct_item_json, "fn_x2", json_mknumber(sct_o[i].fn_x2));

		/* append the object to the array */
		json_append_element(sct_o_json, sct_item_json);
	}
	char *sct_o_json_str = json_stringify(sct_o_json, "\t");

	/* Reply with the JSON */
	mg_http_reply(c, 200, "Content-Type: application/json\r\n",
	              sct_o_json_str);

	/* = Cleanup = */
	json_delete(sct_o_json);
	free(sct_o_json_str);
	sct_instance_free(&sct_instance);
	free(sct_o);
}

int
main(int argc, char **argv)
{
	struct mg_mgr         mgr;
	struct mg_connection *c;

	int  to_print_help, s_port;
	char s_http_addr[21] = "http://0.0.0.0:";
	char s_port_str[6];

	/* = Flags = */
	/* default values */
	to_print_help = 0;
	s_port        = 8000;
	/* define flags */
	spl_flags_toggle(&to_print_help, 'h', "help", "Print help");
	spl_flags_int(&s_port, 'p', "port", "Port number to listen from");

	spl_flags_parse(argc, argv);
	executable_path = argv[0];

	/* Check if help option was passed */
	if (to_print_help)
		print_help_exit(stdout, EXIT_SUCCESS);

	/* = Prerequisites = */
	snprintf(s_port_str, 6, "%d", s_port);
	strncat(s_http_addr, s_port_str, 5);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* = Mongoose = */
	mg_log_set("2");
	mg_mgr_init(&mgr);
	if ((c = mg_http_listen(&mgr, s_http_addr, s_handler_fn, NULL)) ==
	    NULL) {
		MG_ERROR(("Cannot listen on %s. Use http://ADDR:PORT or :PORT.",
		          s_http_addr));
		exit(EXIT_FAILURE);
	}

	/* Start infinite event loop */
	MG_INFO(("Starting sltextpad v%s, listening on '%s'", VERSION,
	         s_http_addr));
	while (s_signo == 0)
		mg_mgr_poll(&mgr, 1000);

	/* Clean exit */
	mg_mgr_free(&mgr);
	MG_INFO(("Exiting on signal %d", s_signo));
	return EXIT_SUCCESS;
}
