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

#ifndef CE_OPTPARSE_H
#define CE_OPTPARSE_H

#include <stdbool.h>

#include "cestring.h"
#include "cevector.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_OPTACTION_STORE,       // argument is required (empty by default)
	CE_OPTACTION_STORE_TRUE,  // no argument, only true or false (by default)
	CE_OPTACTION_STORE_FALSE, // no argument, only true (by default) or false
	CE_OPTACTION_COUNT
} ce_optaction;

typedef struct {
	ce_string* name;
	char short_string;
	ce_string* long_string;
	ce_string* arg_string;
	ce_optaction action;
	ce_string* help;
	ce_string* value;
} ce_optoption;

typedef struct {
	ce_string* name;
	ce_vector* options;
} ce_optgroup;

typedef struct {
	ce_string* name;
	ce_string* help;
	ce_string* value;
} ce_optarg;

typedef struct {
	ce_vector* groups;
	ce_vector* args;
	ce_string* help;
	ce_string* error;
} ce_optparse;

// options

extern void ce_optoption_del(ce_optoption* option);

extern bool ce_optoption_value_bool(const ce_optoption* option);
extern int ce_optoption_value_int(const ce_optoption* option);
extern float ce_optoption_value_float(const ce_optoption* option);

// groups

extern void ce_optgroup_del(ce_optgroup* group);

extern ce_optoption* ce_optgroup_find_option(ce_optgroup* group,
												const char* name);

extern ce_optoption*
ce_optgroup_create_option(ce_optgroup* group,
							const char* name,
							char short_string,        /* 0 if none */
							const char* long_string,  /* NULL if none */
							ce_optaction action,
							const char* help,         /* NULL if none */
							const char* default_value /* NULL if none */);

// args

extern void ce_optarg_del(ce_optarg* arg);

// optparse

extern ce_optparse* ce_optparse_new(const char* description /* NULL if none */);
extern void ce_optparse_del(ce_optparse* optparse);

extern ce_optgroup*
ce_optparse_create_group(ce_optparse* optparse,
							const char* name);

extern ce_optarg*
ce_optparse_create_arg(ce_optparse* optparse,
						const char* name,
						const char* help  /* NULL if none */);

extern ce_optoption* ce_optparse_find_option(ce_optparse* optparse,
											const char* group_name,
											const char* option_name);
extern ce_optgroup* ce_optparse_find_group(ce_optparse* optparse,
											const char* name);
extern ce_optarg* ce_optparse_find_arg(ce_optparse* optparse,
											const char* name);

extern bool ce_optparse_parse_args(ce_optparse* optparse,
									int argc, char* argv[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_OPTPARSE_H */
