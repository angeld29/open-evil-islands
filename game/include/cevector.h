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

extern void ce_vector_push_back(ce_vector* vector, void* item);
extern void* ce_vector_pop_back(ce_vector* vector);

extern void ce_vector_remove(ce_vector* vector, int index);
extern void ce_vector_remove_unordered(ce_vector* vector, int index);

extern void ce_vector_clear(ce_vector* vector);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VECTOR_H */
