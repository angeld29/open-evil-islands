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

#ifndef CE_VECTOR_H
#define CE_VECTOR_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	size_t capacity;
	size_t count;
	void** items;
} ce_vector;

extern ce_vector* ce_vector_new(void);
extern ce_vector* ce_vector_new_reserved(size_t capacity);
extern void ce_vector_del(ce_vector* vector);

extern void ce_vector_reserve(ce_vector* vector, size_t capacity);
extern void ce_vector_resize(ce_vector* vector, size_t count);

static inline bool ce_vector_empty(const ce_vector* vector)
{
	return 0 == vector->count;
}

static inline void* ce_vector_back(const ce_vector* vector)
{
	return vector->items[vector->count - 1];
}

extern size_t ce_vector_find(const ce_vector* vector, const void* item);

extern void* ce_vector_pop_front(ce_vector* vector);
extern void ce_vector_push_back(ce_vector* vector, void* item);

static inline void* ce_vector_pop_back(ce_vector* vector)
{
	return vector->items[--vector->count];
}

extern void ce_vector_insert(ce_vector* vector, size_t index, void* item);
extern void ce_vector_remove(ce_vector* vector, size_t index);
extern void ce_vector_remove_all(ce_vector* vector, const void* item);

static inline void ce_vector_remove_unordered(ce_vector* vector, size_t index)
{
	vector->items[index] = vector->items[--vector->count];
}

static inline void ce_vector_clear(ce_vector* vector)
{
	vector->count = 0;
}

extern void ce_vector_for_each(ce_vector* vector, void (*func)());

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VECTOR_H */
