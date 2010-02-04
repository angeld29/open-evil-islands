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
	size_t name_size;
	size_t value_size;
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
	cfg->sections = ce_vector_open();

	char line[128];
	ce_cfgfile_section* section = NULL;

	for (int line_number = 1;
			NULL != fgets(line, sizeof(line), file); ++line_number) {
		ce_strtrim(line);

		size_t line_length = strlen(line);

		if (0 == line_length || ';' == line[0]) {
			continue;
		}

		if ('[' == line[0]) {
			if (']' != line[line_length - 1]) {
				ce_logging_error("cfgfile: expected ']' after '%s', line %d",
														line, line_number);
				return false;
			}

			if (line_length <= 2) {
				ce_logging_error("cfgfile: unnamed section, line %d",
														line_number);
				return false;
			}

			section = ce_alloc(sizeof(ce_cfgfile_section));
			section->name = ce_alloc(section->name_size = line_length + 1);
			ce_strtrim(ce_strmid(section->name, line, 1, line_length - 2));
			section->options = ce_vector_open();
			ce_vector_push_back(cfg->sections, section);
		} else {
			if (NULL == section) {
				ce_logging_error("cfgfile: option '%s' outside of section, "
									"line %d", line, line_number);
				return false;
			}

			char* eq = strchr(line, '=');
			if (NULL == eq) {
				ce_logging_error("cfgfile: expected '=' after '%s', line %d",
														line, line_number);
				return false;
			}

			size_t eq_pos = eq - line;
			ce_cfgfile_option* option = ce_alloc(sizeof(ce_cfgfile_option));
			option->name = ce_alloc(option->name_size = line_length + 1);
			option->value = ce_alloc(option->value_size = line_length + 1);
			ce_strtrim(ce_strleft(option->name, line, eq_pos));
			ce_strtrim(ce_strright(option->value, line, line_length - eq_pos - 1));
			ce_vector_push_back(section->options, option);

			if ('\0' == option->name[0]) {
				ce_logging_error("cfgfile: missing option name '%s', line %d",
														line, line_number);
				return false;
			}

			if ('\0' == option->value[0]) {
				ce_logging_error("cfgfile: missing option value '%s', line %d",
														line, line_number);
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
		return NULL;
	}

	ce_cfgfile* cfg = ce_alloc(sizeof(ce_cfgfile));
	if (NULL == cfg) {
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
			ce_free(section->name, section->name_size);
			for (size_t j = 0, m = ce_vector_count(section->options); j < m; ++j) {
				ce_cfgfile_option* option = ce_vector_get(section->options, j);
				ce_free(option->name, option->name_size);
				ce_free(option->value, option->value_size);
				ce_free(option, sizeof(ce_cfgfile_option));
			}
			ce_vector_close(section->options);
			ce_free(section, sizeof(ce_cfgfile_section));
		}
		ce_vector_close(cfg->sections);
	}

	ce_free(cfg, sizeof(ce_cfgfile));
}

bool ce_cfgfile_has_section(ce_cfgfile* cfg, const char* section)
{
	return false;
}

bool ce_cfgfile_has_option(ce_cfgfile* cfg, const char* section,
											const char* option)
{
	return false;
}

const char* ce_cfgfile_get(ce_cfgfile* cfg, const char* section,
											const char* option)
{
	return NULL;
}
