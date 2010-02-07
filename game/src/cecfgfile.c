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
#include "cevector.h"
#include "cecfgfile.h"

typedef struct {
	char* name;
	char* value;
} ce_cfgfile_option;

typedef struct {
	char* name;
	ce_vector* options;
} ce_cfgfile_section;

struct ce_cfgfile {
	ce_vector* sections;
};

static bool parse_file(ce_cfgfile* cfg, FILE* file)
{
	if (NULL == (cfg->sections = ce_vector_new())) {
		ce_logging_error("cfgfile: could not allocate memory");
		return false;
	}

	const size_t line_size = 128;
	char line[line_size], temp[line_size], temp2[line_size];
	ce_cfgfile_section* section = NULL;

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

			if (NULL == (section = ce_alloc_zero(sizeof(ce_cfgfile_section)))) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			ce_vector_push_back(cfg->sections, section);

			ce_strmid(temp, line, 1, line_length - 2);
			ce_strtrim(temp2, temp);

			if (NULL == (section->name = ce_alloc(strlen(temp2) + 1))) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			strcpy(section->name, temp2);

			if (NULL == (section->options = ce_vector_new())) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}
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

			ce_cfgfile_option* option = ce_alloc_zero(sizeof(ce_cfgfile_option));
			if (NULL == option) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			ce_vector_push_back(section->options, option);

			ce_strleft(temp, line, eq - line);
			ce_strtrim(temp2, temp);

			if (NULL == (option->name = ce_alloc(strlen(temp2) + 1))) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			strcpy(option->name, temp2);

			if ('\0' == option->name[0]) {
				ce_logging_error("cfgfile: line %d: missing "
								"option name: '%s'", line_number, line);
				return false;
			}

			ce_strright(temp, line, line_length - (eq - line) - 1);
			ce_strtrim(temp2, temp);

			if (NULL == (option->value = ce_alloc(strlen(temp2) + 1))) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			strcpy(option->value, temp2);

			if ('\0' == option->value[0]) {
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
		ce_logging_error("cfgfile: could not open file '%s'", path);
		return NULL;
	}

	ce_cfgfile* cfg = ce_alloc(sizeof(ce_cfgfile));
	if (NULL == cfg) {
		ce_logging_error("cfgfile: could not allocate memory");
		fclose(file);
		return NULL;
	}

	if (!parse_file(cfg, file)) {
		ce_logging_error("cfgfile: failed to parse '%s'", path);
		ce_cfgfile_close(cfg);
		fclose(file);
		return NULL;
	}

	fclose(file);
	return cfg;
}

void ce_cfgfile_close(ce_cfgfile* cfg)
{
	if (NULL == cfg) {
		return;
	}

	if (NULL != cfg->sections) {
		for (size_t i = 0, n = ce_vector_count(cfg->sections); i < n; ++i) {
			ce_cfgfile_section* section = ce_vector_at(cfg->sections, i);
			for (size_t j = 0, m = ce_vector_count(section->options); j < m; ++j) {
				ce_cfgfile_option* option = ce_vector_at(section->options, j);
				if (NULL != option->value) {
					ce_free(option->value, strlen(option->value) + 1);
				}
				if (NULL != option->name) {
					ce_free(option->name, strlen(option->name) + 1);
				}
				ce_free(option, sizeof(ce_cfgfile_option));
			}
			ce_vector_delete(section->options);
			if (NULL != section->name) {
				ce_free(section->name, strlen(section->name) + 1);
			}
			ce_free(section, sizeof(ce_cfgfile_section));
		}
		ce_vector_delete(cfg->sections);
	}

	ce_free(cfg, sizeof(ce_cfgfile));
}

int ce_cfgfile_section_index(ce_cfgfile* cfg, const char* section_name)
{
	for (int i = 0, n = ce_vector_count(cfg->sections); i < n; ++i) {
		ce_cfgfile_section* section = ce_vector_at(cfg->sections, i);
		if (0 == strcmp(section_name, section->name)) {
			return i;
		}
	}
	return -1;
}

int ce_cfgfile_option_index(ce_cfgfile* cfg, int section_index,
											const char* option_name)
{
	ce_cfgfile_section* section = ce_vector_at(cfg->sections, section_index);
	for (int i = 0, n = ce_vector_count(section->options); i < n; ++i) {
		ce_cfgfile_option* option = ce_vector_at(section->options, i);
		if (0 == strcmp(option_name, option->name)) {
			return i;
		}
	}
	return -1;
}

const char* ce_cfgfile_get(ce_cfgfile* cfg, int section_index, int option_index)
{
	ce_cfgfile_section* section = ce_vector_at(cfg->sections, section_index);
	ce_cfgfile_option* option = ce_vector_at(section->options, option_index);
	return option->value;
}
