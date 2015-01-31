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

#include "alloc.hpp"
#include "property.hpp"

ce_property* ce_property_new(const char* name, ce_type type)
{
    ce_property* property = ce_alloc(sizeof(ce_property));
    property->name = ce_string_new_str(name);
    property->value = ce_value_new(type);
    return property;
}

void ce_property_del(ce_property* property)
{
    if (NULL != property) {
        ce_value_del(property->value);
        ce_string_del(property->name);
        ce_free(property, sizeof(ce_property));
    }
}
