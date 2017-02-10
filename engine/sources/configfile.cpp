/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "alloc.hpp"
#include "logging.hpp"
#include "configfile.hpp"

namespace cursedearth
{
    char* ce_strleft(char* dst, const char* src, size_t n)
    {
        n = std::min(n, strlen(src));
        strncpy(dst, src, n);
        dst[n] = '\0';
        return dst;
    }

    char* ce_strright(char* dst, const char* src, size_t n)
    {
        size_t len = strlen(src);
        return strcpy(dst, src + len - std::min(n, len));
    }

    char* ce_strmid(char* dst, const char* src, size_t pos, size_t n)
    {
        size_t len = strlen(src);
        if (pos > len) {
            return NULL;
        }
        n = std::min(n, len - pos);
        strncpy(dst, src + pos, n);
        dst[n] = '\0';
        return dst;
    }

    char* ce_strtrim(char* dst, const char* src)
    {
        size_t len = strlen(src);
        if (0 != len) {
            size_t first, last;
            for (first = 0; first < len && isspace(src[first]); ++first) {
            }
            for (last = len - 1; last > 0 && isspace(src[last]); --last) {
            }
            len = first <= last ? last - first + 1 : 0;
            strncpy(dst, src + first, len);
        }
        dst[len] = '\0';
        return dst;
    }

    bool ce_config_file_parse(ce_config_file* config_file, const char* path, FILE* file)
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

                section = (ce_config_section*)ce_alloc(sizeof(ce_config_section));
                section->name = ce_string_new_str(ce_strtrim(temp2, temp1));
                section->options = ce_vector_new();
                ce_vector_push_back(config_file->sections, section);
            } else {
                if (NULL == section) {
                    // skip comments on top of the file
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
                    option = (ce_config_option*)ce_alloc(sizeof(ce_config_option));
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

    ce_config_file* ce_config_file_open(const boost::filesystem::path& path)
    {
        FILE* file = fopen(path.string().c_str(), "rt");
        if (NULL == file) {
            ce_logging_error("config file: could not open file `%s'", path.string().c_str());
            return NULL;
        }

        ce_config_file* config_file = (ce_config_file*)ce_alloc(sizeof(ce_config_file));
        config_file->sections = ce_vector_new();

        if (!ce_config_file_parse(config_file, path.string().c_str(), file)) {
            ce_logging_error("config file: failed to parse `%s'", path.string().c_str());
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
                ce_config_section* section = (ce_config_section*)config_file->sections->items[i];
                for (size_t j = 0; j < section->options->count; ++j) {
                    ce_config_option* option = (ce_config_option*)section->options->items[j];
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
            ce_config_section* section = (ce_config_section*)config_file->sections->items[i];
            if (0 == strcmp(section_name, section->name->str)) {
                return i;
            }
        }
        return config_file->sections->count;
    }

    size_t ce_config_file_option_index(ce_config_file* config_file, size_t section_index, const char* option_name)
    {
        ce_config_section* section = (ce_config_section*)config_file->sections->items[section_index];
        for (size_t i = 0; i < section->options->count; ++i) {
            ce_config_option* option = (ce_config_option*)section->options->items[i];
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
}
