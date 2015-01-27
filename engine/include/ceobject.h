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

#ifndef CE_OBJECT_H
#define CE_OBJECT_H

#include <stdbool.h>

#include "cevector.h"
#include "cestring.h"
#include "ceproperty.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ce_string* name;
    ce_vector* properties;
} ce_object;

extern ce_object* ce_object_new(const char* name);
extern void ce_object_del(ce_object* object);

extern ce_property* ce_object_find(ce_object* object, const char* name);

static inline bool ce_object_exists(ce_object* object, const char* name)
{
    return NULL != ce_object_find(object, name);
}

static inline void ce_object_add(ce_object* object, ce_property* property)
{
    ce_vector_push_back(object->properties, property);
}

static inline void ce_object_remove(ce_object* object, ce_property* property)
{
    ce_vector_remove_all(object->properties, property);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_OBJECT_H */
