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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "celogging.h"
#include "ceregistry.h"

char* ce_registry_get_string_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name)
{
    if (0 == size) {
        return NULL;
    }

    HKEY hkey;
    DWORD type = REG_SZ;

    LPBYTE data = (LPBYTE)value;
    DWORD data_size = size;

    switch (key) {
    case CE_REGISTRY_KEY_CURRENT_USER:
        hkey = HKEY_CURRENT_USER;
        break;
    case CE_REGISTRY_KEY_LOCAL_MACHINE:
        hkey = HKEY_LOCAL_MACHINE;
        break;
    }

    // see MSDN RegQueryValueEx
    memset(value, '\0', size);

    if (ERROR_SUCCESS == RegOpenKeyEx(hkey, key_name, 0, KEY_READ, &hkey)) {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, value_name, NULL, &type, data, &data_size)) {
            if (REG_SZ == type || REG_EXPAND_SZ == type || REG_MULTI_SZ == type) {
                if (0 == data_size) {
                    value[0] = '\0';
                } else if (data_size < size) {
                    value[data_size] = '\0';
                } else if ('\0' != value[size - 1]) {
                    value[size - 1] = '\0';
                    ce_logging_warning("registry: key `%s': value `%s': truncation occured: `%s'", key_name, value_name, value);
                }
                RegCloseKey(hkey);
                return value;
            }
        }
        RegCloseKey(hkey);
    }

    return NULL;
}

char* ce_registry_get_path_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name)
{
    return ce_registry_get_string_value(value, size, key, key_name, value_name);
}
