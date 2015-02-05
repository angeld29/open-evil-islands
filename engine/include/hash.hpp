/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_HASH_HPP
#define CE_HASH_HPP

#include <cstddef>

namespace cursedearth
{
    typedef unsigned long int ce_hash_key;

    typedef struct ce_hash_entry {
        ce_hash_key hash_key;
        void* value;
        struct ce_hash_entry* next;
    } ce_hash_entry;

    typedef struct {
        size_t item_count;
        size_t entry_count;
        void (*item_dtor)(void*);
        ce_hash_entry** entries;
    } ce_hash;

    ce_hash* ce_hash_new(size_t entry_count, void (*item_dtor)());
    void ce_hash_del(ce_hash* hash);

    inline bool ce_hash_is_empty(const ce_hash* hash)
    {
        return 0 == hash->item_count;
    }

    void ce_hash_insert(ce_hash* hash, ce_hash_key hash_key, void* value);
    void ce_hash_remove(ce_hash* hash, ce_hash_key hash_key);

    void* ce_hash_find(const ce_hash* hash, ce_hash_key hash_key);

    void ce_hash_for_each(ce_hash* hash, void (*func)());
    void ce_hash_for_each_arg1(ce_hash* hash, void (*func)(), void* arg);
    void ce_hash_for_each_key(ce_hash* hash, void (*func)(ce_hash_key));
}

#endif
