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

#ifndef CE_REGISTRY_HPP
#define CE_REGISTRY_HPP

#include <cstddef>

namespace cursedearth
{
    typedef enum {
        CE_REGISTRY_KEY_CURRENT_USER,
        CE_REGISTRY_KEY_LOCAL_MACHINE,
    } ce_registry_key;

    char* ce_registry_get_string_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name);
    char* ce_registry_get_path_value(char* value, size_t size, ce_registry_key key, const char* key_name, const char* value_name);
}

#endif
