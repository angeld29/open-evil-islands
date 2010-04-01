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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

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
		ce_string_del(option->long_string);
		ce_string_del(option->name);
		ce_free(option, sizeof(ce_optoption));
	}
}

bool ce_optoption_value_empty(const ce_optoption* option)
{
	return ce_string_empty(option->value);
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
	option->action = action;
	option->help = ce_string_new_str(NULL != help ? help : "");
	option->value = ce_string_new_str(NULL != default_value ? default_value :
							(const char* []){ "", "false", "true" }[action]);
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

ce_optparse* ce_optparse_new(int version_major, int version_minor,
							int version_patch, const char* description, ...)
{
	ce_optparse* optparse = ce_alloc(sizeof(ce_optparse));
	optparse->version_major = version_major;
	optparse->version_minor = version_minor;
	optparse->version_patch = version_patch;
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
		va_list args;
		va_start(args, description);
		ce_string_append_v(optparse->help, description, args);
		va_end(args);
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

static int ce_optparse_space_count(int max_length, int length, int delim_count)
{
	return ce_max(delim_count, max_length - length + delim_count);
}

static int ce_optparse_append_spaces(ce_string* string, int max_length,
										int length, int delim_count)
{
	int space_count = ce_optparse_space_count(max_length, length, delim_count);
	char spaces[space_count];
	memset(spaces, ' ', space_count);
	return ce_string_append_n(string, spaces, space_count);
}

static int ce_optparse_short_length(ce_optoption* option)
{
	int length = 4; // initial four spaces
	if ('\0' != option->short_string) {
		length += 2; // minus and short string
		if (CE_OPTACTION_STORE == option->action) {
			length += 1 + option->name->length; // space and name
		}
		if (!ce_string_empty(option->long_string)) {
			length += 2; // comma and space
		}
	}
	return length;
}

static int ce_optparse_long_length(ce_optoption* option)
{
	int length = 0;
	if (!ce_string_empty(option->long_string)) {
		length += 2 + option->long_string->length; // --long
		if (CE_OPTACTION_STORE == option->action) {
			length += 1 + option->name->length; // =name
		}
	}
	return length;
}

bool ce_optparse_parse_args(ce_optparse* optparse, int argc, char* argv[])
{
	// first find max short option string (for pretty output)
	int max_short_length = 0;

	for (int i = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
		for (int j = 0; j < group->options->count; ++j) {
			max_short_length = ce_max(max_short_length,
				ce_optparse_short_length(group->options->items[j]));
		}
	}

	// initialize short options, count long options and
	// find max long option string (for pretty output)
	int max_long_length = 0, long_option_count = 0;

	char short_options[128];
	ce_strlcpy(short_options, ":hv", sizeof(short_options));

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

			int length = ce_optparse_short_length(option);
			length += ce_optparse_space_count(max_short_length, length, 0);
			length += ce_optparse_long_length(option);
			length += ce_optparse_space_count(max_long_length, length, 0);
			max_long_length = ce_max(max_long_length, length);
		}
	}

	struct option long_options[long_option_count + 3];

	// some options hard-coded
	long_options[long_option_count].name = "help";
	long_options[long_option_count].has_arg = no_argument;
	long_options[long_option_count].flag = NULL;
	long_options[long_option_count].val = 'h';

	long_options[long_option_count + 1].name = "version";
	long_options[long_option_count + 1].has_arg = no_argument;
	long_options[long_option_count + 1].flag = NULL;
	long_options[long_option_count + 1].val = 'v';

	// terminate the array with an element containing all zeros
	memset(&long_options[long_option_count + 2], '\0', sizeof(struct option));

	ce_string_append(optparse->help, "usage: ");
	ce_string_append(optparse->help, argv[0]);
	ce_string_append(optparse->help, " [options] args\n\noptions:\n");

	// add default help option string
	ce_string_append(optparse->help, "  -h, --help      "
									"show this help message and exit\n");

	// add default version option string
	ce_string_append(optparse->help, "  -v, --version   "
									"display program version\n\n");

	int option_index, long_option_index = 0;

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

			int length = ce_string_append(optparse->help, "    ");
			if ('\0' != option->short_string) {
				length += ce_string_append_f(optparse->help, "-%c",
												option->short_string);
				if (CE_OPTACTION_STORE == option->action) {
					length += ce_string_append_f(optparse->help, " %s",
													option->name->str);
				}
				if (!ce_string_empty(option->long_string)) {
					length += ce_string_append(optparse->help, ", ");
				}
			}
			length += ce_optparse_append_spaces(optparse->help,
												max_short_length, length, 0);

			if (!ce_string_empty(option->long_string)) {
				length += ce_string_append_f(optparse->help, "--%s",
												option->long_string->str);
				if (CE_OPTACTION_STORE == option->action) {
					length += ce_string_append_f(optparse->help, "=%s",
													option->name->str);
				}
			}

			// output help with word wrap
			const char* help_str = option->help->str;
			int help_length = option->help->length;

			if (0 == help_length) {
				ce_string_append(optparse->help, "\n");
			} else {
				while (help_length > 0) {
					ce_optparse_append_spaces(optparse->help,
												max_long_length, length, 3);
					// limit output string at max 79 characters
					int line_length = ce_max(1,
						79 - ce_optparse_space_count(max_long_length, 0, 3));
					if (help_length > line_length) {
						// adjust word wrap
						if (isspace(help_str[line_length])) {
							// the last word is in string boundaries;
							// don't be afraid to output extra spaces
							// beyond possible screen boundaries
							while (line_length < help_length &&
									isspace(help_str[line_length])) {
								++line_length;
							}
						} else {
							// find previous space, current word will be
							// output in next string
							for (int l = line_length - 1;
									l >= 0 && !isspace(help_str[l]); --l) {
								--line_length;
							}
							line_length = ce_max(1, line_length);
						}
					}
					ce_string_append_n(optparse->help, help_str, line_length);
					ce_string_append(optparse->help, "\n");
					help_str += line_length;
					help_length -= line_length;
					length = 0; // line feed, reset length
				}
			}
		}
		ce_string_append(optparse->help, "\n");
	}

	int max_arg_length = 0;

	// find max arg name's length (for pretty output)
	for (int i = 0; i < optparse->args->count; ++i) {
		ce_optarg* arg = optparse->args->items[i];
		max_arg_length = ce_max(max_arg_length, arg->name->length);
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
			ce_optparse_append_spaces(optparse->help, max_arg_length,
										arg->name->length, 3);
			ce_string_append(optparse->help, arg->help->str);
			ce_string_append(optparse->help, "\n");
		}
	}

	int ch;
	opterr = 0;

	while (-1 != (ch = getopt_long(argc, argv, short_options,
								long_options, &long_option_index)))  {
		switch (ch) {
		case 'h':
			ce_logging_write(optparse->help->str);
			return false;
		case 'v':
			ce_logging_write("%d.%d.%d", optparse->version_major,
										optparse->version_minor,
										optparse->version_patch);
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

		// final step for option - assign value
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

	// final step for arg - assign value
	for (int i = 0; i < optparse->args->count; ++i) {
		ce_optarg* arg = optparse->args->items[i];
		ce_string_assign(arg->value, argv[i + optind]);
	}

	return true;
}
