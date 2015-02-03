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

#include <cstring>

#include "alloc.hpp"
#include "object.hpp"

ce_object* ce_object_new(const char* name)
{
    ce_object* object = ce_alloc_zero(sizeof(ce_object));
    object->name = ce_string_new_str(name);
    object->properties = ce_vector_new();
    return object;
}

void ce_object_del(ce_object* object)
{
    if (NULL != object) {
        ce_vector_for_each(object->properties, (void(*)(void*))ce_property_del);
        ce_vector_del(object->properties);
        ce_string_del(object->name);
        ce_free(object, sizeof(ce_object));
    }
}

ce_property* ce_object_find(ce_object* object, const char* name)
{
    for (size_t i = 0; i < object->properties->count; ++i) {
        ce_property* property = object->properties->items[i];
        if (0 == strcmp(name, property->name->str)) {
            return property;
        }
    }
    return NULL;
}
