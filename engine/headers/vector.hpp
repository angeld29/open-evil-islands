/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_VECTOR_HPP
#define CE_VECTOR_HPP

#include <cstddef>

namespace cursedearth
{
    typedef struct {
        size_t capacity;
        size_t count;
        void** items;
    } ce_vector;

    ce_vector* ce_vector_new(void);
    ce_vector* ce_vector_new_reserved(size_t capacity);
    void ce_vector_del(ce_vector* vector);

    void ce_vector_reserve(ce_vector* vector, size_t capacity);
    void ce_vector_resize(ce_vector* vector, size_t count);

    inline bool ce_vector_empty(const ce_vector* vector)
    {
        return 0 == vector->count;
    }

    inline void* ce_vector_back(const ce_vector* vector)
    {
        return vector->items[vector->count - 1];
    }

    size_t ce_vector_find(const ce_vector* vector, const void* item);

    void* ce_vector_pop_front(ce_vector* vector);
    void ce_vector_push_back(ce_vector* vector, void* item);

    inline void* ce_vector_pop_back(ce_vector* vector)
    {
        return vector->items[--vector->count];
    }

    void ce_vector_insert(ce_vector* vector, size_t index, void* item);
    void ce_vector_remove(ce_vector* vector, size_t index);
    void ce_vector_remove_all(ce_vector* vector, const void* item);

    inline void ce_vector_remove_unordered(ce_vector* vector, size_t index)
    {
        vector->items[index] = vector->items[--vector->count];
    }

    inline void ce_vector_clear(ce_vector* vector)
    {
        vector->count = 0;
    }

    void ce_vector_for_each(ce_vector* vector, void (*)(void*));
}

#endif
