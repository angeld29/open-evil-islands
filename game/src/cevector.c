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

#include "cealloc.h"
#include "cevector.h"

ce_vector* ce_vector_new(void)
{
	return ce_vector_new_reserved(16);
}

ce_vector* ce_vector_new_reserved(int capacity)
{
	ce_vector* vector = ce_alloc_zero(sizeof(ce_vector));
	ce_vector_reserve(vector, capacity);
	return vector;
}

void ce_vector_del(ce_vector* vector)
{
	if (NULL != vector) {
		ce_free(vector->items, sizeof(void*) * vector->capacity);
		ce_free(vector, sizeof(ce_vector));
	}
}

void ce_vector_reserve(ce_vector* vector, int capacity)
{
	if (capacity > vector->capacity) {
		void** items = ce_alloc(sizeof(void*) * capacity);
		memcpy(items, vector->items, sizeof(void*) * vector->count);
		ce_free(vector->items, sizeof(void*) * vector->capacity);
		vector->capacity = capacity;
		vector->items = items;
	}
}

bool ce_vector_empty(const ce_vector* vector)
{
	return 0 == vector->count;
}

void* ce_vector_back(const ce_vector* vector)
{
	return vector->items[vector->count - 1];
}

int ce_vector_find(const ce_vector* vector, const void* item)
{
	for (int i = 0; i < vector->count; ++i) {
		if (item == vector->items[i]) {
			return i;
		}
	}
	return -1;
}

void ce_vector_push_back(ce_vector* vector, void* item)
{
	if (vector->count == vector->capacity) {
		ce_vector_reserve(vector, 2 * vector->capacity);
	}
	vector->items[vector->count++] = item;
}

void* ce_vector_pop_back(ce_vector* vector)
{
	return vector->items[--vector->count];
}

void ce_vector_remove(ce_vector* vector, int index)
{
	for (int i = index + 1, n = vector->count--; i < n; ++i) {
		vector->items[i - 1] = vector->items[i];
	}
}

void ce_vector_remove_unordered(ce_vector* vector, int index)
{
	vector->items[index] = vector->items[--vector->count];
}

void ce_vector_clear(ce_vector* vector)
{
	vector->count = 0;
}

void ce_vector_for_each(ce_vector* vector, ce_vector_func1 func)
{
	for (int i = 0; i < vector->count; ++i) {
		(*func)(vector->items[i]);
	}
}
