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

#include <assert.h>

#include "registry.hpp"

char* ce_registry_get_string_value(char* CE_UNUSED(value), size_t CE_UNUSED(size), ce_registry_key CE_UNUSED(key), const char* CE_UNUSED(key_name), const char* CE_UNUSED(value_name))
{
    return NULL;
}

char* ce_registry_get_path_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name)
{
    return ce_registry_get_string_value(value, size, key, key_name, value_name);
}
