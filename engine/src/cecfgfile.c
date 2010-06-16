/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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
#include "cealloc.h"
#include "celogging.h"
#include "cecfgfile.h"

static bool ce_config_file_parse(ce_config_file* config_file, FILE* file)
{
	const size_t max_line_size = 128;
	char line[max_line_size], temp[max_line_size], temp2[max_line_size];
	ce_config_section* section = NULL;

	for (int line_number = 1; NULL != fgets(temp, max_line_size, file); ++line_number) {
		size_t line_length = strlen(ce_strtrim(line, temp));

		if (line_length + 1 == max_line_size) {
			ce_logging_error("config file: line %d: "
							"line is too long: '%s'", line_number, line);
			return false;
		}

		if (0 == line_length || ';' == line[0]) {
			continue;
		}

		if ('[' == line[0]) {
			if (']' != line[line_length - 1]) {
				ce_logging_error("config file: line %d: "
								"expected ']': '%s'", line_number, line);
				return false;
			}

			if (line_length <= 2) {
				ce_logging_error("config file: line %d: "
								"unnamed section: '%s'", line_number, line);
				return false;
			}

			ce_strmid(temp, line, 1, line_length - 2);

			section = ce_alloc(sizeof(ce_config_section));
			section->name = ce_string_new_str(ce_strtrim(temp2, temp));
			section->options = ce_vector_new();
			ce_vector_push_back(config_file->sections, section);
		} else {
			if (NULL == section) {
				ce_logging_error("config file: line %d: option outside of "
								"any section: '%s'", line_number, line);
				return false;
			}

			char* eq = strchr(line, '=');
			if (NULL == eq) {
				ce_logging_error("config file: line %d: "
								"expected '=': '%s'", line_number, line);
				return false;
			}

			ce_config_option* option = ce_alloc(sizeof(ce_config_option));
			option->name = ce_string_new();
			option->value = ce_string_new();
			ce_vector_push_back(section->options, option);

			ce_strleft(temp, line, eq - line);
			ce_string_assign(option->name, ce_strtrim(temp2, temp));

			if (ce_string_empty(option->name)) {
				ce_logging_error("config file: line %d: missing "
								"option name: '%s'", line_number, line);
				return false;
			}

			ce_strright(temp, line, line_length - (eq - line) - 1);
			ce_string_assign(option->value, ce_strtrim(temp2, temp));

			if (ce_string_empty(option->value)) {
				ce_logging_error("config file: line %d: missing "
								"option value: '%s'", line_number, line);
				return false;
			}
		}
	}

	return true;
}

ce_config_file* ce_config_file_open(const char* path)
{
	FILE* file = fopen(path, "rt");
	if (NULL == file) {
		ce_logging_error("config file: could not open file '%s'", path);
		return NULL;
	}

	ce_config_file* config_file = ce_alloc(sizeof(ce_config_file));
	config_file->sections = ce_vector_new();

	if (!ce_config_file_parse(config_file, file)) {
		ce_logging_error("config file: failed to parse '%s'", path);
		ce_config_file_close(config_file);
		fclose(file);
		return NULL;
	}

	fclose(file);
	return config_file;
}

void ce_config_file_close(ce_config_file* config_file)
{
	if (NULL != config_file) {
		for (size_t i = 0; i < config_file->sections->count; ++i) {
			ce_config_section* section = config_file->sections->items[i];
			for (size_t j = 0; j < section->options->count; ++j) {
				ce_config_option* option = section->options->items[j];
				ce_string_del(option->value);
				ce_string_del(option->name);
				ce_free(option, sizeof(ce_config_option));
			}
			ce_vector_del(section->options);
			ce_string_del(section->name);
			ce_free(section, sizeof(ce_config_section));
		}
		ce_vector_del(config_file->sections);
		ce_free(config_file, sizeof(ce_config_file));
	}
}

size_t ce_config_file_section_index(ce_config_file* config_file,
									const char* section_name)
{
	for (size_t i = 0; i < config_file->sections->count; ++i) {
		ce_config_section* section = config_file->sections->items[i];
		if (0 == strcmp(section_name, section->name->str)) {
			return i;
		}
	}
	return config_file->sections->count;
}

size_t ce_config_file_option_index(ce_config_file* config_file,
									size_t section_index,
									const char* option_name)
{
	ce_config_section* section = config_file->sections->items[section_index];
	for (size_t i = 0; i < section->options->count; ++i) {
		ce_config_option* option = section->options->items[i];
		if (0 == strcmp(option_name, option->name->str)) {
			return i;
		}
	}
	return section->options->count;
}

const char* ce_config_file_find(ce_config_file* config_file,
								const char* section_name,
								const char* option_name)
{
	size_t section_index = ce_config_file_section_index(config_file, section_name);
	if (section_index != ce_config_file_section_count(config_file)) {
		size_t option_index = ce_config_file_option_index(config_file, section_index, option_name);
		if (option_index != ce_config_file_option_count(config_file, section_index)) {
			return ce_config_file_get(config_file, section_index, option_index);
		}
	}
	return NULL;
}
