/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include <string.h>
#include <assert.h>

#include "cealloc.h"
#include "ceobject.h"

ce_object* ce_object_new(void)
{
	ce_object* object = ce_alloc(sizeof(ce_object));
	object->properties = ce_vector_new();
	return object;
}

void ce_object_del(ce_object* object)
{
	if (NULL != object) {
		ce_vector_for_each(object->properties, ce_property_del);
		ce_vector_del(object->properties);
		ce_free(object, sizeof(ce_object));
	}
}

bool ce_object_exists(ce_object* object, const char* name)
{
	return NULL != ce_object_find(object, name);
}

void ce_object_add(ce_object* object, ce_property* property)
{
	ce_vector_push_back(object->properties, property);
}

void ce_object_remove(ce_object* object, ce_property* property)
{
	ce_vector_remove_all(object->properties, property);
}

ce_property* ce_object_find(ce_object* object, const char* name)
{
	for (int i = 0; i < object->properties->count; ++i) {
		ce_property* property = object->properties->items[i];
		if (0 == strcmp(name, property->name->str)) {
			return property;
		}
	}
	return NULL;
}