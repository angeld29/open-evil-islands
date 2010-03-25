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

ce_optparse* ce_optparse_new(void)
{
	ce_optparse* optparse = ce_alloc(sizeof(ce_optparse));
	optparse->groups = ce_vector_new();
	optparse->args = ce_vector_new();
	optparse->help = ce_string_new();
	return optparse;
}

void ce_optparse_del(ce_optparse* optparse)
{
	if (NULL != optparse) {
		ce_vector_for_each(optparse->args, (ce_vector_func1)ce_string_del);
		ce_vector_for_each(optparse->groups, (ce_vector_func1)ce_optgroup_del);
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

ce_optoption* ce_optparse_find_option(ce_optparse* optparse,
										const char* group_name,
										const char* option_name)
{
	ce_optgroup* group = ce_optparse_find_group(optparse, group_name);
	return NULL != group ? ce_optgroup_find_option(group, option_name) : NULL;
}

const char* ce_optparse_value(ce_optparse* optparse,
								const char* group_name,
								const char* option_name)
{
	for (int i = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
		if (0 == ce_strcasecmp(group_name, group->name->str)) {
			for (int j = 0; j < group->options->count; ++j) {
				ce_optoption* option = group->options->items[j];
				if (0 == ce_strcasecmp(option_name, option->name->str)) {
					return option->value->str;
				}
			}
		}
	}
	return NULL;
}

bool ce_optparse_parse_args(ce_optparse* optparse, int argc, char* argv[])
{
	int ch, option_index, long_option_index = 0, long_option_count = 0;

	char short_options[256], short_option[2] = { [1] = '\0' };
	ce_strlcpy(short_options, ":h", sizeof(short_options));

	// initialize short options and count long options
	for (int i = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
		for (int j = 0; j < group->options->count; ++j) {
			ce_optoption* option = group->options->items[j];
			if ('\0' != option->short_string) {
				short_option[0] = option->short_string;
				ce_strlcat(short_options, short_option, sizeof(short_options));
				if (CE_OPTACTION_STORE == option->action) {
					ce_strlcat(short_options, ":", sizeof(short_options));
				}
			}
			if (!ce_string_empty(option->long_string)) {
				++long_option_count;
			}
		}
	}

	struct option long_options[long_option_count + 1];
	memset(&long_options[long_option_count], '\0', sizeof(struct option));

	// initialize long options
	for (int i = 0, k = 0; i < optparse->groups->count; ++i) {
		ce_optgroup* group = optparse->groups->items[i];
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
			ce_logging_error("optparse: option '-%c' "
							"requires an argument", optopt);
			return false;
		case '?':
			ce_logging_error("optparse: unknown option '-%c'", optopt);
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

		const char* values[CE_OPTACTION_COUNT] = {
			optarg, "true", "false"
		};

		// final step - assign value
		for (int i = 0, k = 0; i < optparse->groups->count; ++i) {
			ce_optgroup* group = optparse->groups->items[i];
			for (int j = 0; j < group->options->count; ++j, ++k) {
				ce_optoption* option = group->options->items[j];
				if (k == option_index) {
					ce_string_assign(option->value, values[option->action]);
				}
			}
		}
	}

	for (int i = optind; i < argc; ++i) {
		ce_vector_push_back(optparse->args, ce_string_new_str(argv[i]));
	}

	return true;
}
