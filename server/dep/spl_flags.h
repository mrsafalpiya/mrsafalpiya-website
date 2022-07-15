/*
 ===============================================================================
 |                                 spl_flags.h                                 |
 |                      https://github.com/mrsafalpiya/spl                     |
 |                                                                             |
 |             GNU-style argument parsing inspired by golang's flag            |
 |                                                                             |
 |                  No warranty implied; Use at your own risk                  |
 |                   See end of file for license information.                  |
 ===============================================================================
 */

/*
 * https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html
 */

/*
 ===============================================================================
 |                                    Usage                                    |
 ===============================================================================
 *
 * Just include this header file and you are done.
 *
 * See the example code below and check out the options.
 */

/*
 ===============================================================================
 |                                 Example code                                |
 ===============================================================================
 */

#if 0
#include <stdio.h>
#include <stdlib.h>

#include "dep/spl_flags.h"

int
main(int argc, char **argv)
{
	int            to_print_help, age;
	float          gpa;
	char          *univ, *name;
	spl_flags_info f_info;

	/* Initialize values on flags */
	to_print_help = 0;
	age           = 20;
	gpa           = 3.6;
	univ          = "Tribhuvan University";
	name          = NULL;

	/* Set flags */
	spl_flags_toggle(&to_print_help, 'h', "help", "Print the help message");
	spl_flags_int(&age, 'a', NULL, "Your age");
	spl_flags_float(&gpa, ' ', "gpa", "Your gpa");
	spl_flags_str(&univ, 'u', "university", "Your university");

	f_info = spl_flags_parse(argc, argv);

	/* Checking for an empty valued flag */
	if (f_info.non_value_flag != NULL) {
		fprintf(stderr, "No value given on the flag '%s'\n",
		        f_info.non_value_flag);
		exit(EXIT_FAILURE);
	}

	/* Check if -h flag was passed */
	if (to_print_help) {
		fprintf(stdout, "Usage: %s name\n\nAvailable options are:\n",
		        argv[0]);
		spl_flags_print_help(stdout);
		exit(EXIT_SUCCESS);
	}

	/* Check if the user gave us a name as an argument */
	if (f_info.non_flag_arguments_c == 0) {
		fprintf(stderr, "Usage: %s name\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	name = f_info.non_flag_arguments[0];

	printf("Your name is %s aged %d studying in %s and you got %.2f gpa.\n",
	       name, age, univ, gpa);

	return 0;
}
#endif

/*
 * $ ./bin/spl-test
 * Usage: ./bin/spl-test name
 *
 * $ ./bin/spl-test -h
 * Usage: ./bin/spl-test name
 *
 * Available options are:
 *     -h, --help, (Default: Off)  Print the help message
 *     -a, (Default: 20)   Your age
 *     --gpa, (Default: 3.60)      Your gpa
 *     -u, --university, (Default: 'Tribhuvan University') Your university
 *
 * $ ./bin/spl-test Safal
 * Your name is Safal aged 20 studying in Tribhuvan University and you got 3.60
 * gpa.
 *
 * $ ./bin/spl-test Safal -a=21 --gpa 3.2 -u "Kathmandu University"
 * Your name is Safal aged 21 studying in Kathmandu University and you got 3.20
 * gpa.
 *
 * $ ./bin/spl-test Safal -a=21 --gpa 3.2 -u
 * No value given on the flag 'u'
 */

/*
 ===============================================================================
 |                                   Options                                   |
 ===============================================================================
 */

/* = SPL_FLAGS = */
#ifndef SPL_FLAGS_DEF
#define SPL_FLAGS_DEF /* You may want `static` or `static inline` here */
#endif

#define SPL_FLAGS_MAX 256 /* Max number of flags to support */

#ifndef SPL_FLAGS_FLOAT_PRECISION
#define SPL_FLAGS_FLOAT_PRECISION "2" /* Float precision on 'print_help()' */
#endif

/* values on toggle */
#ifndef SPL_FLAGS_TOGGLE_0_STR
#define SPL_FLAGS_TOGGLE_0_STR \
	"Off" /* String representation for toggle 0 on 'print_help()' */
#endif

#ifndef SPL_FLAGS_TOGGLE_1_STR
#define SPL_FLAGS_TOGGLE_1_STR \
	"On" /* String representation for toggle 1 on 'print_help()' */
#endif

/*
 ===============================================================================
 |                       HEADER-FILE/IMPLEMENTATION MODE                       |
 ===============================================================================
 */

#ifndef SPL_FLAGS_H
#define SPL_FLAGS_H

#include <string.h>
#include <stdlib.h>

/*
 ===============================================================================
 |                                     Data                                    |
 ===============================================================================
 */

/* = Enums = */
typedef enum {
	SPL_FLAG_TYPE_TOGGLE,
	SPL_FLAG_TYPE_INT,
	SPL_FLAG_TYPE_FLOAT,
	SPL_FLAG_TYPE_STR
} spl_flag_type;

typedef enum {
	NONE,
	SHORT_NON_EQUAL,
	SHORT_EQUAL,
	LONG_NON_EQUAL,
	LONG_EQUAL
} spl_flags_arg_type;

/* = Unions = */
typedef union {
	int   tog_num;
	float float_num;
	char  str[1024];
} spl_flags_def_value;

/* = Structs = */
typedef struct {
	spl_flag_type       type;
	char                short_hand;
	char                long_hand[256];
	spl_flags_def_value def_value;
	char                info[512];
	void               *data_ptr;
} spl_flag_entry;

typedef struct {
	char *non_defined_flags_long[256];
	char  non_defined_flags_short[256];
	char *non_flag_arguments[256];
	int   non_defined_flags_long_c;
	int   non_defined_flags_short_c;
	int   non_flag_arguments_c;
	char *non_value_flag;
} spl_flags_info;

/* = Global variables = */
static spl_flag_entry spl_flags[SPL_FLAGS_MAX];
static int            spl_flags_c = 0;

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */

SPL_FLAGS_DEF void
spl_flags_toggle(int *f_toggle, const char short_hand, const char *long_hand,
                 const char *info);
/* Creates a toggle-type flag. */

SPL_FLAGS_DEF void
spl_flags_int(int *f_int, const char short_hand, const char *long_hand,
              const char *info);
/* Creates a int-type flag. */

SPL_FLAGS_DEF void
spl_flags_float(float *f_float, const char short_hand, const char *long_hand,
                const char *info);
/* Creates a float-type flag. */

SPL_FLAGS_DEF void
spl_flags_str(char **f_str, const char short_hand, const char *long_hand,
              const char *info);
/* Creates a string-type flag. */

SPL_FLAGS_DEF spl_flags_info
spl_flags_parse(int argc, char **argv);
/*
 * Parses all the flags with the given argc and argv.
 *
 * Make sure to pass the argc and argv from the 'main' without any
 * modifications.
 */

SPL_FLAGS_DEF void
spl_flags_print_help(FILE *stream);
/* Outputs the help message to the given `stream`. */

/*
 ===============================================================================
 |                           Function Implementations                          |
 ===============================================================================
 */

#define SPL_FLAGS_ADD_MACRO                                          \
	spl_flags[spl_flags_c].short_hand = short_hand;              \
	if (long_hand != NULL)                                       \
		strcpy(spl_flags[spl_flags_c].long_hand, long_hand); \
	else                                                         \
		spl_flags[spl_flags_c].long_hand[0] = '\0';          \
	strcpy(spl_flags[spl_flags_c].info, info);                   \
	++spl_flags_c;
SPL_FLAGS_DEF void
spl_flags_toggle(int *f_toggle, const char short_hand, const char *long_hand,
                 const char *info)
{
	spl_flags[spl_flags_c].type              = SPL_FLAG_TYPE_TOGGLE;
	spl_flags[spl_flags_c].data_ptr          = (void *)f_toggle;
	spl_flags[spl_flags_c].def_value.tog_num = *f_toggle;
	SPL_FLAGS_ADD_MACRO
}

SPL_FLAGS_DEF void
spl_flags_int(int *f_int, const char short_hand, const char *long_hand,
              const char *info)
{
	spl_flags[spl_flags_c].type              = SPL_FLAG_TYPE_INT;
	spl_flags[spl_flags_c].data_ptr          = (void *)f_int;
	spl_flags[spl_flags_c].def_value.tog_num = *f_int;
	SPL_FLAGS_ADD_MACRO
}

SPL_FLAGS_DEF void
spl_flags_float(float *f_float, const char short_hand, const char *long_hand,
                const char *info)
{
	spl_flags[spl_flags_c].type                = SPL_FLAG_TYPE_FLOAT;
	spl_flags[spl_flags_c].data_ptr            = (void *)f_float;
	spl_flags[spl_flags_c].def_value.float_num = *f_float;
	SPL_FLAGS_ADD_MACRO
}

SPL_FLAGS_DEF void
spl_flags_str(char **f_str, const char short_hand, const char *long_hand,
              const char *info)
{
	spl_flags[spl_flags_c].type     = SPL_FLAG_TYPE_STR;
	spl_flags[spl_flags_c].data_ptr = (void **)f_str;
	if (*f_str != NULL)
		strcpy(spl_flags[spl_flags_c].def_value.str, *f_str);

	SPL_FLAGS_ADD_MACRO
}

SPL_FLAGS_DEF spl_flags_info
spl_flags_parse(int argc, char **argv)
{
	int                i, j, is_double_dash, is_long_arg;
	char              *cur_arg, *equal_ch;
	spl_flags_info     f_info;
	spl_flags_arg_type a_type;

	is_double_dash = 0;
	cur_arg        = NULL;

	f_info.non_defined_flags_long_c = 0;
	f_info.non_flag_arguments_c     = 0;
	f_info.non_value_flag           = NULL;

	for (i = 1; i < argc; i++) {
		is_long_arg = 0;
		equal_ch    = NULL;

		/* Check if double dash was present or the current argv is NOT a
		 * flag type argument */
		if (is_double_dash || argv[i][0] != '-') {
			f_info.non_flag_arguments[f_info.non_flag_arguments_c++] =
				argv[i];
			continue;
		}

		/* Check for double dash */
		if (strcmp(argv[i], "--") == 0) {
			is_double_dash = 1;
			continue;
		}

		/* = Now we have a valid flag argument = */
		cur_arg = argv[i] + 1;

		/* Check if long or short type argument */
		if (cur_arg[0] == '-') {
			is_long_arg = 1;
			++cur_arg;
		}

	flag_proc: /* I really have to use goto */
		/* Check for equal sign */
		equal_ch = (cur_arg[1] == '=') ? cur_arg + 1 : NULL;
		if (is_long_arg)
			equal_ch = strchr(cur_arg, '=');

		/* Iterate through defined flags and search for valid flag */
		for (j = 0; j < spl_flags_c; j++) {
			/* Get the argument type */
			a_type = NONE;

			if (is_long_arg) {
				if (equal_ch &&
				    ((strncmp(cur_arg, spl_flags[j].long_hand,
				              equal_ch - cur_arg)) == 0))
					a_type = LONG_EQUAL;
				else if ((strcmp(cur_arg,
				                 spl_flags[j].long_hand)) == 0)
					a_type = LONG_NON_EQUAL;
			} else {
				if (cur_arg[0] == spl_flags[j].short_hand) {
					if (equal_ch)
						a_type = SHORT_EQUAL;
					else
						a_type = SHORT_NON_EQUAL;
				}
			}

			if (a_type == NONE)
				continue;

			/* Perform proper action according to argument type */
			switch (a_type) {
			case LONG_EQUAL:
			case SHORT_EQUAL:
				switch (spl_flags[j].type) {
				case SPL_FLAG_TYPE_TOGGLE:
					*((int *)spl_flags[j].data_ptr) =
						atoi(equal_ch + 1);
					break;
				case SPL_FLAG_TYPE_INT:
					*((int *)spl_flags[j].data_ptr) =
						atoi(equal_ch + 1);
					break;
				case SPL_FLAG_TYPE_FLOAT:
					*((float *)spl_flags[j].data_ptr) =
						atof(equal_ch + 1);
					break;
				case SPL_FLAG_TYPE_STR:
					*((char **)spl_flags[j].data_ptr) =
						equal_ch + 1;
					break;
				}
				break;
			case LONG_NON_EQUAL:
			case SHORT_NON_EQUAL:
				switch (spl_flags[j].type) {
				case SPL_FLAG_TYPE_TOGGLE:
					*((int *)spl_flags[j].data_ptr) = !*(
						(int *)spl_flags[j].data_ptr);
					/* Check if other arguments can be
					 * parsed after cur_arg but in the
					 * current argv */
					if (a_type == SHORT_NON_EQUAL &&
					    (*(cur_arg + 1) != '\0')) {
						++cur_arg;
						equal_ch = (cur_arg[1] == '=') ?
						                   cur_arg + 1 :
                                                                   NULL;
						continue;
					}
					break;
				case SPL_FLAG_TYPE_INT:
					if (i == argc - 1) {
						f_info.non_value_flag = cur_arg;
						return f_info; /* TODO: Should I return? */
					}

					if (a_type == SHORT_NON_EQUAL &&
					    *(cur_arg + 1) != '\0') {
						*((int *)spl_flags[j].data_ptr) =
							atoi(cur_arg + 1);
						break;
					}

					*((int *)spl_flags[j].data_ptr) =
						atoi(argv[++i]);
					break;
				case SPL_FLAG_TYPE_FLOAT:
					if (i == argc - 1) {
						f_info.non_value_flag = cur_arg;
						return f_info;
					}

					if (a_type == SHORT_NON_EQUAL &&
					    *(cur_arg + 1) != '\0') {
						*((float *)spl_flags[j]
						          .data_ptr) =
							atof(cur_arg + 1);
						break;
					}

					*((float *)spl_flags[j].data_ptr) =
						atof(argv[++i]);
					break;
				case SPL_FLAG_TYPE_STR:
					if (i == argc - 1) {
						f_info.non_value_flag = cur_arg;
						return f_info;
					}

					if (a_type == SHORT_NON_EQUAL &&
					    *(cur_arg + 1) != '\0') {
						*((char **)spl_flags[j]
						          .data_ptr) =
							cur_arg + 1;
						break;
					}

					*((char **)spl_flags[j].data_ptr) =
						argv[++i];
					break;
				}
				break;
			case NONE:
				break;
			}

			if (a_type != NONE)
				break;
		}

		/* Check if the argument was an undefined flag */
		if (a_type == NONE) {
			if (is_long_arg)
				f_info.non_defined_flags_long
					[f_info.non_defined_flags_long_c++] =
					cur_arg;
			else {
				f_info.non_defined_flags_short
					[f_info.non_defined_flags_short_c++] =
					*cur_arg;
				/* Check if other arguments can be parsed after
				 * cur_arg but in the current argv */
				if (*(cur_arg + 1) != '\0') {
					++cur_arg;
					goto flag_proc;
					continue;
				}
			}
		}
	}

	return f_info;
}

SPL_FLAGS_DEF void
spl_flags_print_help(FILE *stream)
{
	int i;

	for (i = 0; i < spl_flags_c; i++) {
		fprintf(stream, "    ");

		/* flag value */
		if (spl_flags[i].short_hand != ' ')
			fprintf(stream, "-%c, ", spl_flags[i].short_hand);
		if (spl_flags[i].long_hand[0] != '\0')
			fprintf(stream, "--%s, ", spl_flags[i].long_hand);

		/* default value */
		switch (spl_flags[i].type) {
		case SPL_FLAG_TYPE_TOGGLE:
			fprintf(stream, "(Default: %s)",
			        spl_flags[i].def_value.tog_num == 0 ?
			                SPL_FLAGS_TOGGLE_0_STR :
                                        SPL_FLAGS_TOGGLE_1_STR);
			break;
		case SPL_FLAG_TYPE_INT:
			fprintf(stream, "(Default: %d)",
			        spl_flags[i].def_value.tog_num);
			break;
		case SPL_FLAG_TYPE_FLOAT:
			fprintf(stream,
			        "(Default: %0." SPL_FLAGS_FLOAT_PRECISION "f)",
			        spl_flags[i].def_value.float_num);
			break;
		case SPL_FLAG_TYPE_STR:
			if (spl_flags[i].def_value.str[0] != '\0')
				fprintf(stream, "(Default: '%s')",
				        spl_flags[i].def_value.str);
		}

		/* info */
		if (spl_flags[i].info[0] != '\0')
			fprintf(stream, "\t%s", spl_flags[i].info);

		fprintf(stream, "\n");
	}
}

#endif /* SPL_FLAGS_H */

/*
 ===============================================================================
 |                 License - Public Domain (www.unlicense.org)                 |
 ===============================================================================
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */
