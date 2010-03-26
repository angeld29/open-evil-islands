/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ok, portable
// getopt is available on all POSIX-compliant systems and on Windows (MinGW)
#include <getopt.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptparse.h"

void ce_optoption_del(ce_optoption* option)
{
	if (NULL != option) {
		ce_string_del(option->value);
		ce_string_del(option->help);
		ce_string_del(option->arg_string);
		ce_string_del(option->long_string);
		ce_string_del(option->name);
		ce_free(option, sizeof(ce_optoption));
	}
}

bool ce_optoption_value_bool(const ce_optoption* option)
{
	return 0 == ce_strcasecmp("true", option->value->str);
}

int ce_optoption_value_int(const ce_optoption* option)
{
	return atoi(option->value->str);
}

float ce_optoption_value_float(const ce_optoption* option)
{
	return atof(option->value->str);
}

void ce_optgroup_del(ce_optgroup* group)
{
	if (NULL != group) {
		ce_vector_for_each(group->options, (ce_vector_func1)ce_optoption_del);
		ce_vector_del(group->options);
		ce_string_del(group->name);
		ce_free(group, sizeof(ce_optgroup));
	}
}

ce_optoption* ce_optgroup_find_option(ce_optgroup* group, const char* name)
{
	for (int i = 0; i < group->options->count; ++i) {
		ce_optoption* option = group->options->items[i];
		if (0 == ce_strcasecmp(name, option->name->str)) {
			return option;
		}
	}
	return NULL;
}

ce_optoption* ce_optgroup_create_option(ce_optgroup* group,
										const char* name,
										char short_string,
										const char* long_string,
										ce_optaction action,
										const char* help,
										const char* default_value)
{
	ce_optoption* option = ce_alloc(sizeof(ce_optoption));
	option->name = ce_string_new_str(name);
	option->short_string = short_string;
	option->long_string = ce_string_new_str(NULL != long_string ? long_string : "");
	option->arg_string = ce_string_new_str_n("    ", 32);
	option->action = action;
	option->help = ce_string_new_str(NULL != help ? help : "");
	option->value = ce_string_new_str(NULL != default_value ? default_value :
							(const char* []){ "", "false", "true" }[action]);

	char name_uppercase[option->name->length + 1];
	ce_strupr(name_uppercase, option->name->str);

	if ('\0' != short_string) {
		if (CE_OPTACTION_STORE == action) {
			ce_string_append_f(option->arg_string, "-%c %s", short_string,
															name_uppercase);
		} else {
			ce_string_append_f(option->arg_string, "-%c", short_string);
		}
		if (NULL != long_string) {
			ce_string_append(option->arg_string, ", ");
		}
	}

	if (NULL != long_string) {
		if (CE_OPTACTION_STORE == action) {
			ce_string_append_f(option->arg_string, "--%s=%s", long_string,
															name_uppercase);
		} else {
			ce_string_append_f(option->arg_string, "--%s", long_string);
		}
	}

	ce_vector_push_back(group->options, option);
	return option;
}

void ce_optarg_del(ce_optarg* arg)
{
	if (NULL != arg) {
		ce_string_del(arg->value);
		ce_string_del(arg->help);
		ce_string_del(arg->name);
		ce_free(arg, sizeof(ce_optarg));
	}
}

ce_optparse* ce_optparse_new(const char* description)
{
	ce_optparse* optparse = ce_alloc(sizeof(ce_optparse));
	optparse->groups = ce_vector_new();
	optparse->args = ce_vector_new();
	optparse->help = ce_string_new_reserved(2048); // extra space for help
	optparse->error = ce_string_new();

	ce_string_append(optparse->help, "\n"
		"===============================================================================\n"
		"Cursed Earth is an open source, cross-platform port of Evil Islands.\n"
		"Copyright (C) 2009-2010 Yanis Kurganov.\n\n"
		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 3 of the License, or\n"
		"(at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"GNU General Public License for more details.\n"
		"===============================================================================\n\n");

	if (NULL != description) {
		ce_string_append(optparse->help, description);
		ce_string_append(optparse->help, "\n\n");
	}

	return optparse;
}

void ce_optparse_del(ce_optparse* optparse)
{
	if (NULL != optparse) {
		ce_vector_for_each(optparse->args, (ce_vector_func1)ce_optarg_del);
		ce_vector_for_each(optparse->groups, (ce_vector_func1)ce_optgroup_del);
		ce_string_del(optparse->error);
		ce_string_del(optparse->help);
		ce_vector_del(optparse->args);
		ce_vector_del(optparse->groups);
		ce_free(optparse, sizeof(ce_optparse));
	}
}

ce_optgroup* ce_optparse_create_group(ce_optparse* optparse, const char* name)
{
	ce_optgroup* group = ce_alloc(sizeof(ce_optgroup));
	group->name = ce_string_new_str(name);
	group->options = ce_vector_new();
	ce_vector_push_back(optparse->groups, group);
	return group;
}

ce_optarg* ce_optparse_create_arg(ce_optparse* optparse,
									const char* name,
									const char* help)
{
	ce_optarg* arg = ce_alloc(sizeof(ce_optarg));
	arg->name = ce_string_new_str(name);
	arg->help = ce_string_new_str(NULL != help ? help : "");
	arg->value = ce_string_new();
	ce_vector_push_back(optparse->args, arg);
	return arg;
}

ce_optoption* ce_optparse_find_option(ce_optparse* optparse,
										const char* group_name,
										const char* option_name)
{
	ce_optgroup* group = ce_optparse_find_group(optparse, group_name);
	return NULL != group ? ce_optgroup_find_option(group, option_name) : NULL;
}

ce_optgroup* ce_optparse_find_group(ce_optparse* optparse, const char* name)
{
	for (int i = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
		if (0 == ce_strcasecmp(name, group->name->str)) {
			return group;
		}
	}
	return NULL;
}

ce_optarg* ce_optparse_find_arg(ce_optparse* optparse, const char* name)
{
	for (int i = 0; i < optparse->args->count; ++i) {
		ce_optarg* arg = optparse->args->items[i];
		if (0 == ce_strcasecmp(name, arg->name->str)) {
			return arg;
		}
	}
	return NULL;
}

static void ce_string_append_spaces(ce_string* string,
									int max_length, int length)
{
	int space_count = ce_max(3, max_length - length + 3);
	char spaces[space_count];
	memset(spaces, ' ', space_count);
	ce_string_append_n(string, spaces, space_count);
}

bool ce_optparse_parse_args(ce_optparse* optparse, int argc, char* argv[])
{
	int option_index, long_option_index = 0, long_option_count = 0;
	int ch, max_length = 0;

	char short_options[256];
	ce_strlcpy(short_options, ":h", sizeof(short_options));

	// initialize short options, count long options and
	// find max option string's length (for pretty output)
	for (int i = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
		for (int j = 0; j < group->options->count; ++j) {
			ce_optoption* option = group->options->items[j];
			if ('\0' != option->short_string) {
				char short_option[2] = { option->short_string, '\0' };
				ce_strlcat(short_options, short_option, sizeof(short_options));
				if (CE_OPTACTION_STORE == option->action) {
					ce_strlcat(short_options, ":", sizeof(short_options));
				}
			}
			if (!ce_string_empty(option->long_string)) {
				++long_option_count;
			}
			max_length = ce_max(max_length, option->arg_string->length);
		}
	}

	struct option long_options[long_option_count + 2];

	// special long option - help
	long_options[long_option_count].name = "help";
	long_options[long_option_count].has_arg = no_argument;
	long_options[long_option_count].flag = NULL;
	long_options[long_option_count].val = 'h';

	// terminate the array with an element containing all zeros
	memset(&long_options[long_option_count + 1], '\0', sizeof(struct option));

	ce_string_append(optparse->help, "usage: ");
	ce_string_append(optparse->help, argv[0]);
	ce_string_append(optparse->help, " [options] args\n\noptions:\n");

	// add default help message
	const char* help_arg_string = "  -h, --help";
	ce_string_append(optparse->help, help_arg_string);
	ce_string_append_spaces(optparse->help, max_length, strlen(help_arg_string));
	ce_string_append(optparse->help, "show this help message and exit\n\n");

	// initialize long options and generate options help
	for (int i = 0, k = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
		ce_string_append(optparse->help, "  ");
		ce_string_append(optparse->help, group->name->str);
		ce_string_append(optparse->help, ":\n");
		for (int j = 0; j < group->options->count; ++j, ++k) {
			ce_optoption* option = group->options->items[j];
			if (!ce_string_empty(option->long_string)) {
				long_options[long_option_index].name = option->long_string->str;
				long_options[long_option_index].has_arg =
					CE_OPTACTION_STORE == option->action ? required_argument :
															no_argument;
				long_options[long_option_index].flag =
					option->short_string ? NULL : &option_index;
				long_options[long_option_index++].val =
					option->short_string ? option->short_string : k;
			}
			ce_string_append(optparse->help, option->arg_string->str);
			ce_string_append_spaces(optparse->help, max_length,
									option->arg_string->length);
			ce_string_append(optparse->help, option->help->str);
			ce_string_append(optparse->help, "\n");
		}
		ce_string_append(optparse->help, "\n");
	}

	max_length = 0;

	// find max arg name's length (for pretty output)
	for (int i = 0; i < optparse->args->count; ++i) {
		ce_optarg* arg = optparse->args->items[i];
		max_length = ce_max(max_length, arg->name->length);
	}

	ce_string_append(optparse->help, "args:\n");

	// generate args help
	if (ce_vector_empty(optparse->args)) {
		ce_string_append(optparse->help, "  none\n");
	} else {
		for (int i = 0; i < optparse->args->count; ++i) {
			ce_optarg* arg = optparse->args->items[i];
			ce_string_append(optparse->help, "  ");
			ce_string_append(optparse->help, arg->name->str);
			ce_string_append_spaces(optparse->help, max_length,
													arg->name->length);
			ce_string_append(optparse->help, arg->help->str);
			ce_string_append(optparse->help, "\n");
		}
	}

	opterr = 0;

	while (-1 != (ch = getopt_long(argc, argv, short_options,
								long_options, &long_option_index)))  {
		switch (ch) {
		case 'h':
			ce_logging_write(optparse->help->str);
			return false;
		case ':':
			ce_string_append(optparse->help, "\n");
			ce_string_assign_f(optparse->error, "optparse: option '-%c' "
												"requires an argument", optopt);
			ce_logging_write(optparse->help->str);
			ce_logging_error(optparse->error->str);
			return false;
		case '?':
			ce_string_append(optparse->help, "\n");
			ce_string_assign_f(optparse->error, "optparse: unknown "
												"option '-%c'", optopt);
			ce_logging_write(optparse->help->str);
			ce_logging_error(optparse->error->str);
			return false;
		}

		if (0 == ch) {
			// long option, short option is missing
			// option_index was already initialized by getopt
		} else {
			// short option is present, find it
			for (int i = 0, k = 0; i < optparse->groups->count; ++i) {
				ce_optgroup* group = optparse->groups->items[i];
				for (int j = 0; j < group->options->count; ++j, ++k) {
					ce_optoption* option = group->options->items[j];
					if (ch == option->short_string) {
						option_index = k;
					}
				}
			}
		}

		// final step - assign value
		for (int i = 0, k = 0; i < optparse->groups->count; ++i) {
			ce_optgroup* group = optparse->groups->items[i];
			for (int j = 0; j < group->options->count; ++j, ++k) {
				ce_optoption* option = group->options->items[j];
				if (k == option_index) {
					ce_string_assign(option->value, (const char* [])
						{ optarg, "true", "false" }[option->action]);
				}
			}
		}
	}

	if (argc - optind != optparse->args->count) {
		ce_string_append(optparse->help, "\n");
		ce_string_assign(optparse->error, argc - optind > optparse->args->count ?
									"optparse: too much non-option arguments:\n" :
									"optparse: too few non-option arguments:\n");
		if (argc == optind) {
			ce_string_append(optparse->error, "  none\n");
		} else {
			for (int i = optind; i < argc; ++i) {
				ce_string_append_f(optparse->error, "  %s\n", argv[i]);
			}
		}
		ce_logging_write(optparse->help->str);
		ce_logging_error(optparse->error->str);
		return false;
	}

	for (int i = 0; i < optparse->args->count; ++i) {
		ce_optarg* arg = optparse->args->items[i];
		ce_string_assign(arg->value, argv[i + optind]);
	}

	return true;
}
