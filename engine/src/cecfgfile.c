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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "cestr.h"
#include "celogging.h"
#include "cealloc.h"
#include "cecfgfile.h"

static bool ce_cfgfile_parse(ce_cfgfile* cfg, FILE* file)
{
	const size_t line_size = 128;
	char line[line_size], temp[line_size], temp2[line_size];
	ce_cfgsection* section = NULL;

	for (int line_number = 1;
			NULL != fgets(temp, line_size, file); ++line_number) {
		size_t line_length = strlen(ce_strtrim(line, temp));

		if (line_length + 1 == line_size) {
			ce_logging_error("cfgfile: line %d: "
							"line is too long: '%s'", line_number, line);
			return false;
		}

		if (0 == line_length || ';' == line[0]) {
			continue;
		}

		if ('[' == line[0]) {
			if (']' != line[line_length - 1]) {
				ce_logging_error("cfgfile: line %d: "
								"expected ']': '%s'", line_number, line);
				return false;
			}

			if (line_length <= 2) {
				ce_logging_error("cfgfile: line %d: "
								"unnamed section: '%s'", line_number, line);
				return false;
			}

			ce_strmid(temp, line, 1, line_length - 2);

			section = ce_alloc(sizeof(ce_cfgsection));
			section->name = ce_string_new_str(ce_strtrim(temp2, temp));
			section->options = ce_vector_new();
			ce_vector_push_back(cfg->sections, section);
		} else {
			if (NULL == section) {
				ce_logging_error("cfgfile: line %d: option outside of "
								"any section: '%s'", line_number, line);
				return false;
			}

			char* eq = strchr(line, '=');
			if (NULL == eq) {
				ce_logging_error("cfgfile: line %d: "
								"expected '=': '%s'", line_number, line);
				return false;
			}

			ce_cfgoption* option = ce_alloc(sizeof(ce_cfgoption));
			option->name = ce_string_new();
			option->value = ce_string_new();
			ce_vector_push_back(section->options, option);

			ce_strleft(temp, line, eq - line);
			ce_string_assign(option->name, ce_strtrim(temp2, temp));

			if (ce_string_empty(option->name)) {
				ce_logging_error("cfgfile: line %d: missing "
								"option name: '%s'", line_number, line);
				return false;
			}

			ce_strright(temp, line, line_length - (eq - line) - 1);
			ce_string_assign(option->value, ce_strtrim(temp2, temp));

			if (ce_string_empty(option->value)) {
				ce_logging_error("cfgfile: line %d: missing "
								"option value: '%s'", line_number, line);
				return false;
			}
		}
	}

	return true;
}

ce_cfgfile* ce_cfgfile_open(const char* path)
{
	FILE* file = fopen(path, "rt");
	if (NULL == file) {
		ce_logging_error("cfgfile: could not open file: '%s'", path);
		return NULL;
	}

	ce_cfgfile* cfg = ce_alloc(sizeof(ce_cfgfile));
	cfg->sections = ce_vector_new();

	if (!ce_cfgfile_parse(cfg, file)) {
		ce_logging_error("cfgfile: failed to parse file: '%s'", path);
		ce_cfgfile_close(cfg);
		fclose(file);
		return NULL;
	}

	fclose(file);
	return cfg;
}

void ce_cfgfile_close(ce_cfgfile* cfg)
{
	if (NULL != cfg) {
		for (size_t i = 0; i < cfg->sections->count; ++i) {
			ce_cfgsection* sec = cfg->sections->items[i];
			for (size_t j = 0; j < sec->options->count; ++j) {
				ce_cfgoption* opt = sec->options->items[j];
				ce_string_del(opt->value);
				ce_string_del(opt->name);
				ce_free(opt, sizeof(ce_cfgoption));
			}
			ce_vector_del(sec->options);
			ce_string_del(sec->name);
			ce_free(sec, sizeof(ce_cfgsection));
		}
		ce_vector_del(cfg->sections);
		ce_free(cfg, sizeof(ce_cfgfile));
	}
}

int ce_cfgfile_section_index(ce_cfgfile* cfg, const char* section_name)
{
	for (size_t i = 0; i < cfg->sections->count; ++i) {
		ce_cfgsection* sec = cfg->sections->items[i];
		if (0 == strcmp(section_name, sec->name->str)) {
			return i;
		}
	}
	return -1;
}

int ce_cfgfile_option_index(ce_cfgfile* cfg, int section_index,
											const char* option_name)
{
	ce_cfgsection* sec = cfg->sections->items[section_index];
	for (size_t i = 0; i < sec->options->count; ++i) {
		ce_cfgoption* opt = sec->options->items[i];
		if (0 == strcmp(option_name, opt->name->str)) {
			return i;
		}
	}
	return -1;
}

const char* ce_cfgfile_get(ce_cfgfile* cfg, int section_index, int option_index)
{
	ce_cfgsection* sec = cfg->sections->items[section_index];
	ce_cfgoption* opt = sec->options->items[option_index];
	return opt->value->str;
}
