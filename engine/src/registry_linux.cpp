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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "lib.hpp"
#include "str.hpp"
#include "path.hpp"
#include "logging.hpp"
#include "configfile.hpp"
#include "registry.hpp"

namespace cursedearth
{
static void ce_registry_append_quotes(char* dst, size_t size, const char* src)
{
    if (NULL != dst && NULL != src) {
        ce_strlcpy(dst, "\"", size);
        ce_strlcat(dst, src, size);
        ce_strlcat(dst, "\"", size);
    }
}

static void ce_registry_remove_quotes(char* dst, size_t size, const char* src)
{
    if (NULL != dst && NULL != src) {
        char* left = strchr(src, '"');
        char* right = strrchr(src, '"');
        if (NULL != left && NULL != right && left < right) {
            ce_strmid(dst, src, left - src + 1, right - src - 1);
        } else {
            ce_strlcpy(dst, src, size);
        }
    }
}

static char* ce_registry_find_value(char* value, size_t size, const char* wine_prefix, const char* reg_file_name, const char* key_name, const char* value_name)
{
    char buffer[CE_PATH_MAX];
    ce_path_join(buffer, sizeof(buffer), wine_prefix, reg_file_name, NULL);

    ce_config_file* config_file = ce_config_file_open(buffer);
    if (NULL == config_file) {
        ce_logging_error("registry: could not open file `%s'", buffer);
        return NULL;
    }

    char fixed_key_name[strlen(key_name) + 64];
    ce_strrep(fixed_key_name, key_name, sizeof(fixed_key_name), "\\", "\\\\");

    char fixed_value_name[strlen(value_name) + 8];
    ce_registry_append_quotes(fixed_value_name, sizeof(fixed_value_name), value_name);

    const char* reg_value = ce_config_file_find(config_file, fixed_key_name, fixed_value_name);
    if (NULL == reg_value) {
        ce_logging_warning("registry: could not find %s\\%s", key_name, value_name);
        value = NULL;
    } else {
        ce_registry_remove_quotes(value, size, reg_value);
    }

    ce_config_file_close(config_file);
    return value;
}

static char* ce_registry_find_string_value(char* value, size_t size, const ce_vector* wine_prefixes, const char* reg_file_name, const char* key_name, const char* value_name)
{
    for (size_t i = 0; i < wine_prefixes->count; ++i) {
        ce_string* wine_prefix = wine_prefixes->items[i];
        if (NULL != ce_registry_find_value(value, size, wine_prefix->str, reg_file_name, key_name, value_name)) {
            return value;
        }
    }
    return NULL;
}

static char* ce_registry_find_path_value(char* value, size_t size, const ce_vector* wine_prefixes, const char* reg_file_name, const char* key_name, const char* value_name)
{
    char buffer[CE_PATH_MAX];
    for (size_t i = 0; i < wine_prefixes->count; ++i) {
        ce_string* wine_prefix = wine_prefixes->items[i];
        if (NULL != ce_registry_find_value(buffer, sizeof(buffer), wine_prefix->str, reg_file_name, key_name, value_name)) {
            ce_strrep(buffer, buffer, sizeof(buffer), "\\\\", "/");
            ce_strrep(buffer, buffer, sizeof(buffer), "C:", "drive_c");
            ce_strrep(buffer, buffer, sizeof(buffer), "D:", "drive_d");
            return ce_path_join(value, size, wine_prefix->str, buffer, NULL);
        }
    }
    return NULL;
}

static char* ce_registry_get_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name,
    char* (func)(char*, size_t, const ce_vector*, const char*, const char*, const char*))
{
    const char* home_path = getenv("HOME");
    if (NULL == home_path) {
        ce_logging_error("registry: could not get environment variable `HOME'");
        return NULL;
    }

    const char* reg_file_name;
    switch (key) {
    case CE_REGISTRY_KEY_CURRENT_USER:
        reg_file_name = "user.reg";
        break;
    case CE_REGISTRY_KEY_LOCAL_MACHINE:
        reg_file_name = "system.reg";
        break;
    default:
        ce_logging_error("registry: unknown key");
        return NULL;
    }

    char buffer[CE_PATH_MAX];
    ce_vector* wine_prefixes = ce_vector_new_reserved(1);

    // add .PlayOnLinux prefixes
    ce_path_join(buffer, sizeof(buffer), home_path, ".PlayOnLinux", "wineprefix", NULL);
    ce_path_list_subdirs(buffer, wine_prefixes);

    // add .wine prefix
    ce_path_join(buffer, sizeof(buffer), home_path, ".wine", NULL);
    ce_vector_push_back(wine_prefixes, ce_string_new_str(buffer));

    value = (*func)(value, size, wine_prefixes, reg_file_name, key_name, value_name);

    ce_vector_for_each(wine_prefixes, ce_string_del);
    ce_vector_del(wine_prefixes);

    return value;
}

char* ce_registry_get_string_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name)
{
    return ce_registry_get_value(value, size, key, key_name, value_name, ce_registry_find_string_value);
}

char* ce_registry_get_path_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name)
{
    return ce_registry_get_value(value, size, key, key_name, value_name, ce_registry_find_path_value);
}
}
