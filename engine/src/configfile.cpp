/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>
#include <cstring>

#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "configfile.hpp"

static bool ce_config_file_parse(ce_config_file* config_file, const char* path, FILE* file)
{
    enum {
        MAX_LINE_SIZE = 256,
    };

    char line[MAX_LINE_SIZE], temp1[MAX_LINE_SIZE], temp2[MAX_LINE_SIZE];
    ce_config_section* section = NULL;
    ce_config_option* option = NULL;
    bool was_continuation_character = false;

    for (int line_number = 1; NULL != fgets(temp1, MAX_LINE_SIZE, file); ++line_number) {
        size_t line_length = strlen(ce_strtrim(line, temp1));

        if (line_length + 1 == MAX_LINE_SIZE) {
            ce_logging_warning("config file: %s:%d: line is too long: `%s'", path, line_number, line);
            // FIXME: skip line tail until new line
            int ch;
            do {
                ch = fgetc(file);
            } while (EOF != ch && '\n' != ch);
            continue;
        }

        if (0 == line_length || ';' == line[0]) {
            continue;
        }

        if ('[' == line[0]) {
            char* rbracket = strrchr(line, ']');
            if (NULL == rbracket) {
                ce_logging_error("config file: %s:%d: expected `]': `%s'", path, line_number, line);
                return false;
            }

            // truncate right stuff
            *(rbracket + 1) = '\0';
            line_length = strlen(line);

            if (line_length <= 2) {
                ce_logging_error("config file: %s:%d: unnamed section: `%s'", path, line_number, line);
                return false;
            }

            ce_strmid(temp1, line, 1, line_length - 2);

            section = ce_alloc(sizeof(ce_config_section));
            section->name = ce_string_new_str(ce_strtrim(temp2, temp1));
            section->options = ce_vector_new();
            ce_vector_push_back(config_file->sections, section);
        } else {
            if (NULL == section) {
                ce_logging_warning("config file: %s:%d: option outside of any section: `%s'", path, line_number, line);
                continue;
            }

            char* eq = strchr(line, '=');
            if (NULL == eq && !was_continuation_character) {
                ce_logging_error("config file: %s:%d: expected `=': `%s'", path, line_number, line);
                return false;
            }
            if (NULL != eq && was_continuation_character) {
                ce_logging_error("config file: %s:%d: unexpected `=': `%s'", path, line_number, line);
                return false;
            }

            // find continuation character
            char* bs = strrchr(line, '\\');
            if (NULL != bs) {
                if ('\0' == *(bs + 1)) {
                    // remove continuation character
                    *bs = '\0';
                } else {
                    // it's not a continuation character
                    bs = NULL;
                }
            }

            if (was_continuation_character) {
                ce_string_append(option->value, line);
            } else {
                option = ce_alloc(sizeof(ce_config_option));
                option->name = ce_string_new();
                option->value = ce_string_new();
                ce_vector_push_back(section->options, option);

                ce_strleft(temp1, line, eq - line);
                ce_string_assign(option->name, ce_strtrim(temp2, temp1));

                if (ce_string_empty(option->name)) {
                    ce_logging_warning("config file: %s:%d: missing option name: `%s'", path, line_number, line);
                }

                ce_strright(temp1, line, line_length - (eq - line) - 1);
                ce_string_assign(option->value, ce_strtrim(temp2, temp1));
            }

            if (ce_string_empty(option->value)) {
                ce_logging_warning("config file: %s:%d: missing option value: `%s'", path, line_number, line);
            }

            was_continuation_character = (NULL != bs);
        }
    }

    return true;
}

ce_config_file* ce_config_file_open(const char* path)
{
    FILE* file = fopen(path, "rt");
    if (NULL == file) {
        ce_logging_error("config file: could not open file `%s'", path);
        return NULL;
    }

    ce_config_file* config_file = ce_alloc(sizeof(ce_config_file));
    config_file->sections = ce_vector_new();

    if (!ce_config_file_parse(config_file, path, file)) {
        ce_logging_error("config file: failed to parse `%s'", path);
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

size_t ce_config_file_section_index(ce_config_file* config_file, const char* section_name)
{
    for (size_t i = 0; i < config_file->sections->count; ++i) {
        ce_config_section* section = config_file->sections->items[i];
        if (0 == strcmp(section_name, section->name->str)) {
            return i;
        }
    }
    return config_file->sections->count;
}

size_t ce_config_file_option_index(ce_config_file* config_file, size_t section_index, const char* option_name)
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

const char* ce_config_file_find(ce_config_file* config_file, const char* section_name, const char* option_name)
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
