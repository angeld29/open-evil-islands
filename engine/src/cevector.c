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

#include <assert.h>

#include "cealloc.h"
#include "cevector.h"

ce_vector* ce_vector_new(void)
{
	return ce_vector_new_reserved(16);
}

ce_vector* ce_vector_new_reserved(size_t capacity)
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

void ce_vector_reserve(ce_vector* vector, size_t capacity)
{
	if (capacity > vector->capacity) {
		vector->items = ce_realloc(vector->items,
			sizeof(void*) * vector->capacity, sizeof(void*) * capacity);
		vector->capacity = capacity;
	}
}

void ce_vector_resize(ce_vector* vector, size_t count)
{
	ce_vector_reserve(vector, count);
	vector->count = count;
}

size_t ce_vector_find(const ce_vector* vector, const void* item)
{
	for (size_t i = 0; i < vector->count; ++i) {
		if (item == vector->items[i]) {
			return i;
		}
	}
	return vector->count;
}

void* ce_vector_pop_front(ce_vector* vector)
{
	void* item = vector->items[0];
	ce_vector_remove(vector, 0);
	return item;
}

void ce_vector_push_back(ce_vector* vector, void* item)
{
	if (vector->count == vector->capacity) {
		ce_vector_reserve(vector, vector->capacity << 1);
	}
	vector->items[vector->count++] = item;
}

void ce_vector_insert(ce_vector* vector, size_t index, void* item)
{
	if (vector->count == vector->capacity) {
		ce_vector_reserve(vector, vector->capacity << 1);
	}
	for (size_t i = vector->count++; i > index; --i) {
		vector->items[i] = vector->items[i - 1];
	}
	vector->items[index] = item;
}

void ce_vector_remove(ce_vector* vector, size_t index)
{
	for (size_t i = index + 1, n = vector->count--; i < n; ++i) {
		vector->items[i - 1] = vector->items[i];
	}
}

void ce_vector_remove_all(ce_vector* vector, const void* item)
{
	size_t index;
	while (vector->count != (index = ce_vector_find(vector, item))) {
		ce_vector_remove(vector, index);
	}
}

void ce_vector_for_each(ce_vector* vector, void (*func)(void*))
{
	for (size_t i = 0; i < vector->count; ++i) {
		(*func)(vector->items[i]);
	}
}
