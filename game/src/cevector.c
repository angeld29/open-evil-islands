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

#include "celogging.h"
#include "cealloc.h"
#include "cevector.h"

struct ce_vector {
	size_t capacity;
	size_t count;
	void** items;
};

ce_vector* ce_vector_new(void)
{
	ce_vector* vec = ce_alloc(sizeof(ce_vector));
	if (NULL == vec) {
		ce_logging_error("vector: could not allocate memory");
		return NULL;
	}

	vec->capacity = 16;
	vec->count = 0;

	if (NULL == (vec->items = ce_alloc(sizeof(void*) * vec->capacity))) {
		ce_logging_error("vector: could not allocate memory");
		ce_vector_delete(vec);
		return NULL;
	}

	return vec;
}

void ce_vector_delete(ce_vector* vec)
{
	if (NULL != vec) {
		ce_free(vec->items, sizeof(void*) * vec->capacity);
	}

	ce_free(vec, sizeof(ce_vector));
}

void ce_vector_push_back(ce_vector* vec, void* item)
{
	if (vec->count == vec->capacity) {
		size_t capacity = 2 * vec->capacity;

		void** items = ce_alloc(sizeof(void*) * capacity);
		if (NULL == items) {
			ce_logging_error("vector: could not allocate memory");
			return;
		}

		memcpy(items, vec->items, sizeof(void*) * vec->count);
		ce_free(vec->items, sizeof(void*) * vec->capacity);

		vec->capacity = capacity;
		vec->items = items;
	}

	vec->items[vec->count++] = item;
}

size_t ce_vector_count(ce_vector* vec)
{
	return vec->count;
}

void* ce_vector_get(ce_vector* vec, size_t index)
{
	return vec->items[index];
}
