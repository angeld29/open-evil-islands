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

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef void (*ce_vector_func1)(void* item);

typedef struct {
	int capacity;
	int count;
	void** items;
} ce_vector;

extern ce_vector* ce_vector_new(void);
extern ce_vector* ce_vector_new_reserved(int capacity);
extern void ce_vector_del(ce_vector* vector);

extern void ce_vector_reserve(ce_vector* vector, int capacity);

extern bool ce_vector_empty(const ce_vector* vector);
extern void* ce_vector_back(const ce_vector* vector);

extern int ce_vector_find(const ce_vector* vector, const void* item);

extern void ce_vector_push_back(ce_vector* vector, void* item);
extern void* ce_vector_pop_back(ce_vector* vector);

extern void ce_vector_remove(ce_vector* vector, int index);
extern void ce_vector_remove_unordered(ce_vector* vector, int index);

extern void ce_vector_clear(ce_vector* vector);

extern void ce_vector_for_each(ce_vector* vector, ce_vector_func1 func);

typedef void (*ce_vector_func_proc)();

typedef struct {
	int capacity;
	int count;
	ce_vector_func_proc* items;
} ce_vector_func;

extern ce_vector_func* ce_vector_func_new(void);
extern ce_vector_func* ce_vector_func_new_reserved(int capacity);
extern void ce_vector_func_del(ce_vector_func* vector);

extern void ce_vector_func_reserve(ce_vector_func* vector, int capacity);

extern void ce_vector_func_push_back(ce_vector_func* vector,
									ce_vector_func_proc item);

extern void ce_vector_func_call1(ce_vector_func* vector, void* item);
extern void ce_vector_func_call2(ce_vector_func* vector, void* item1, void* item2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VECTOR_H */
