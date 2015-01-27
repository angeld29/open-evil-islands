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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cestr.h"
#include "cepath.h"
#include "celogging.h"
#include "ceconfigfile.h"
#include "ceregistry.h"

static void ce_registry_replace_separator(char* dst, size_t size, const char* src,
                                                const char* from, const char* to)
{
    char buffer[strlen(src) + 1], *pos = buffer;
    memcpy(buffer, src, sizeof(buffer));

    dst[0] = '\0';

    do {
        char* part = ce_strsep(&pos, from);
        if ('\0' != part[0]) {
            if ('\0' != dst[0]) {
                ce_strlcat(dst, to, size);
            }
            ce_strlcat(dst, part, size);
        }
    } while (NULL != pos);
}

static void ce_registry_append_quotes(char* dst, size_t size, const char* src)
{
    ce_strlcpy(dst, "\"", size);
    ce_strlcat(dst, src, size);
    ce_strlcat(dst, "\"", size);
}

static void ce_registry_remove_quotes(char* dst, size_t size, const char* src)
{
    char* left = strchr(src, '"');
    char* right = strrchr(src, '"');
    if (NULL != left && NULL != right && left < right) {
        ce_strmid(dst, src, left - src + 1, right - src - 1);
    } else {
        ce_strlcpy(dst, src, size);
    }
}

char* ce_registry_get_string_value(char* value, size_t size,
                                    ce_registry_key key,
                                    const char* key_name,
                                    const char* value_name)
{
    const char* home_path = getenv("HOME");
    if (NULL == home_path) {
        ce_logging_error("registry: could not get environment variable 'HOME'");
        return NULL;
    }

    const char* reg_name = "unknown";
    switch (key) {
    case CE_REGISTRY_KEY_CURRENT_USER:
        reg_name = "user.reg";
        break;
    case CE_REGISTRY_KEY_LOCAL_MACHINE:
        reg_name = "system.reg";
        break;
    }

    char path[CE_PATH_MAX];

    if (NULL == ce_path_join(path, sizeof(path),
                            home_path, ".wine", reg_name, NULL)) {
        return NULL;
    }

    ce_config_file* config_file = ce_config_file_open(path);
    if (NULL == config_file) {
        //ce_logging_error("registry: could not open file '%s'", path);
        return NULL;
    }

    char fixed_key_name[strlen(key_name) + 64];
    ce_registry_replace_separator(fixed_key_name, sizeof(fixed_key_name), key_name, "\\", "\\\\");

    char fixed_value_name[strlen(value_name) + 8];
    ce_registry_append_quotes(fixed_value_name, sizeof(fixed_value_name), value_name);

    const char* reg_value = ce_config_file_find(config_file, fixed_key_name, fixed_value_name);
    if (NULL == reg_value) {
        value = NULL;
        ce_logging_warning("registry: could not find %s\\%s", key_name, value_name);
    } else {
        ce_registry_remove_quotes(value, size, reg_value);
    }

    ce_config_file_close(config_file);
    return value;
}
