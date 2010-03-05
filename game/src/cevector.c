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
	return ce_vector_new_reserved(16);
}

ce_vector* ce_vector_new_reserved(size_t capacity)
{
	ce_vector* vec = ce_alloc_zero(sizeof(ce_vector));
	if (NULL == vec) {
		ce_logging_error("vector: could not allocate memory");
		return NULL;
	}

	ce_vector_reserve(vec, capacity);

	return vec;
}

void ce_vector_del(ce_vector* vec)
{
	if (NULL != vec) {
		ce_free(vec->items, sizeof(void*) * vec->capacity);
		ce_free(vec, sizeof(ce_vector));
	}
}

bool ce_vector_reserve(ce_vector* vec, size_t capacity)
{
	if (capacity > vec->capacity) {
		void** items = ce_alloc(sizeof(void*) * capacity);
		if (NULL == items) {
			ce_logging_error("vector: could not allocate memory");
			return false;
		}

		if (NULL != vec->items) {
			memcpy(items, vec->items, sizeof(void*) * vec->count);
			ce_free(vec->items, sizeof(void*) * vec->capacity);
		}

		vec->capacity = capacity;
		vec->items = items;
	}

	return true;
}

size_t ce_vector_count(const ce_vector* vec)
{
	return vec->count;
}

bool ce_vector_empty(const ce_vector* vec)
{
	return 0 == vec->count;
}

void* ce_vector_front(ce_vector* vec)
{
	return vec->items[0];
}

void* ce_vector_back(ce_vector* vec)
{
	return vec->items[vec->count - 1];
}

void* ce_vector_at(ce_vector* vec, int index)
{
	return vec->items[index];
}

bool ce_vector_push_back(ce_vector* vec, void* item)
{
	if (vec->count == vec->capacity) {
		ce_vector_reserve(vec, 2 * vec->capacity);
	}

	vec->items[vec->count++] = item;
	return true;
}

void* ce_vector_pop_back(ce_vector* vec)
{
	return vec->items[--vec->count];
}

void ce_vector_replace(ce_vector* vec, int index, void* item)
{
	vec->items[index] = item;
}

void ce_vector_clear(ce_vector* vec)
{
	vec->count = 0;
}
