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
#include "cevector.h"

struct ce_vector {
	size_t capacity;
	size_t count;
	void** items;
};

ce_vector* ce_vector_open(void)
{
	ce_vector* vec = ce_alloc(sizeof(ce_vector));
	if (NULL == vec) {
		return NULL;
	}

	vec->capacity = 32;
	vec->count = 0;

	if (NULL == (vec->items = ce_alloc(sizeof(void*) * vec->capacity))) {
		ce_vector_close(vec);
		return NULL;
	}

	return vec;
}

void ce_vector_close(ce_vector* vec)
{
	if (NULL == vec) {
		return;
	}

	ce_free(vec->items, sizeof(void*) * vec->capacity);

	ce_free(vec, sizeof(ce_vector));
}

bool ce_vector_push_back(ce_vector* vec, void* item)
{
	assert(vec->count < vec->capacity && "To be implemented");
	vec->items[vec->count] = item;
	++vec->count;
	return true;
}

size_t ce_vector_count(ce_vector* vec)
{
	return vec->count;
}

void* ce_vector_get(ce_vector* vec, size_t index)
{
	return vec->items[index];
}
