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
#include <string.h>

#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "cevector.h"
#include "cecfgfile.h"

typedef struct {
	size_t name_value_size;
	char* name;
	char* value;
} ce_cfgfile_option;

typedef struct {
	size_t name_size;
	char* name;
	ce_vector* options;
} ce_cfgfile_section;

struct ce_cfgfile {
	ce_vector* sections;
};

static bool parse_file(ce_cfgfile* cfg, FILE* file)
{
	if (NULL == (cfg->sections = ce_vector_open())) {
		ce_logging_error("cfgfile: could not allocate memory");
		return false;
	}

	const size_t line_size = 128;
	char line[line_size], temp[line_size];
	ce_cfgfile_section* section = NULL;

	for (int line_number = 1;
			NULL != fgets(temp, line_size, file); ++line_number) {
		ce_strtrim(line, temp);

		size_t line_length = strlen(line);

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

			if (NULL == (section = ce_alloc(sizeof(ce_cfgfile_section)))) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			ce_vector_push_back(cfg->sections, section);

			section->name_size = line_length + 1;
			section->name = ce_alloc(section->name_size);
			section->options = ce_vector_open();

			if (NULL == section->name || NULL == section->options) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			ce_strmid(temp, line, 1, line_length - 2);
			ce_strtrim(section->name, temp);
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

			ce_cfgfile_option* option = ce_alloc(sizeof(ce_cfgfile_option));
			if (NULL == option) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			ce_vector_push_back(section->options, option);

			option->name_value_size = line_length + 1;
			option->name = ce_alloc(option->name_value_size);
			option->value = ce_alloc(option->name_value_size);

			if (NULL == option->name || NULL == option->value) {
				ce_logging_error("cfgfile: could not allocate memory");
				return false;
			}

			ce_strleft(temp, line, eq - line);
			ce_strtrim(option->name, temp);
			ce_strright(temp, line, line_length - (eq - line) - 1);
			ce_strtrim(option->value, temp);

			if ('\0' == option->name[0]) {
				ce_logging_error("cfgfile: line %d: missing "
								"option name: '%s'", line_number, line);
				return false;
			}

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
			ce_cfgfile_section* section = ce_vector_get(cfg->sections, i);
			for (size_t j = 0, m = ce_vector_count(section->options); j < m; ++j) {
				ce_cfgfile_option* option = ce_vector_get(section->options, j);
				ce_free(option->name, option->name_value_size);
				ce_free(option->value, option->name_value_size);
				ce_free(option, sizeof(ce_cfgfile_option));
			}
			ce_vector_close(section->options);
			ce_free(section->name, section->name_size);
			ce_free(section, sizeof(ce_cfgfile_section));
		}
		ce_vector_close(cfg->sections);
	}

	ce_free(cfg, sizeof(ce_cfgfile));
}

bool ce_cfgfile_has_section(ce_cfgfile* cfg, const char* section_name)
{
	for (size_t i = 0, n = ce_vector_count(cfg->sections); i < n; ++i) {
		ce_cfgfile_section* section = ce_vector_get(cfg->sections, i);
		if (0 == strcmp(section_name, section->name)) {
			return true;
		}
	}
	return false;
}

bool ce_cfgfile_has_option(ce_cfgfile* cfg, const char* section_name,
											const char* option_name)
{
	for (size_t i = 0, n = ce_vector_count(cfg->sections); i < n; ++i) {
		ce_cfgfile_section* section = ce_vector_get(cfg->sections, i);
		if (0 == strcmp(section_name, section->name)) {
			for (size_t j = 0, m = ce_vector_count(section->options); j < m; ++j) {
				ce_cfgfile_option* option = ce_vector_get(section->options, j);
				if (0 == strcmp(option_name, option->name)) {
					return true;
				}
			}
		}
	}
	return false;
}

const char* ce_cfgfile_get(ce_cfgfile* cfg, const char* section_name,
											const char* option_name)
{
	for (size_t i = 0, n = ce_vector_count(cfg->sections); i < n; ++i) {
		ce_cfgfile_section* section = ce_vector_get(cfg->sections, i);
		if (0 == strcmp(section_name, section->name)) {
			for (size_t j = 0, m = ce_vector_count(section->options); j < m; ++j) {
				ce_cfgfile_option* option = ce_vector_get(section->options, j);
				if (0 == strcmp(option_name, option->name)) {
					return option->value;
				}
			}
		}
	}
	return NULL;
}
