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

#include "alloc.hpp"
#include "hash.hpp"

static ce_hash_entry* ce_hash_entry_new(ce_hash_key hash_key)
{
    ce_hash_entry* hash_entry = ce_alloc_zero(sizeof(ce_hash_entry));
    hash_entry->hash_key = hash_key;
    return hash_entry;
}

static void ce_hash_entry_del(ce_hash_entry* hash_entry, void (*item_dtor)(void*))
{
    if (NULL != hash_entry) {
        if (NULL != item_dtor) {
            (*item_dtor)(hash_entry->value);
        }
        ce_free(hash_entry, sizeof(ce_hash_entry));
    }
}

static void ce_hash_entry_del_cascade(ce_hash_entry* hash_entry, void (*item_dtor)(void*))
{
    if (NULL != hash_entry) {
        ce_hash_entry_del_cascade(hash_entry->next, item_dtor);
        ce_hash_entry_del(hash_entry, item_dtor);
    }
}

static void ce_hash_entry_for_each(ce_hash_entry* hash_entry, void (*func)(void*))
{
    if (NULL != hash_entry) {
        (*func)(hash_entry->value);
        ce_hash_entry_for_each(hash_entry->next, func);
    }
}

static void ce_hash_entry_for_each_arg1(ce_hash_entry* hash_entry, void (*func)(void*, void*), void* arg)
{
    if (NULL != hash_entry) {
        (*func)(hash_entry->value, arg);
        ce_hash_entry_for_each_arg1(hash_entry->next, func, arg);
    }
}

static void ce_hash_entry_for_each_key(ce_hash_entry* hash_entry, void (*func)(ce_hash_key))
{
    if (NULL != hash_entry) {
        (*func)(hash_entry->hash_key);
        ce_hash_entry_for_each_key(hash_entry->next, func);
    }
}

ce_hash* ce_hash_new(size_t entry_count, void (*item_dtor)())
{
    ce_hash* hash = ce_alloc_zero(sizeof(ce_hash) + sizeof(ce_hash_entry*) * entry_count);
    hash->entry_count = entry_count;
    hash->item_dtor = item_dtor;
    return hash;
}

void ce_hash_del(ce_hash* hash)
{
    if (NULL != hash) {
        for (size_t i = 0; i < hash->entry_count; ++i) {
            ce_hash_entry_del_cascade(hash->entries[i], hash->item_dtor);
        }
        ce_free(hash, sizeof(ce_hash) + sizeof(ce_hash_entry*) * hash->entry_count);
    }
}

void ce_hash_insert(ce_hash* hash, ce_hash_key hash_key, void* value)
{
    size_t index = hash_key % hash->entry_count;

    ce_hash_entry* hash_entry = ce_hash_entry_new(hash_key);
    hash_entry->value = value;
    hash_entry->next = hash->entries[index];

    hash->entries[index] = hash_entry;
    ++hash->item_count;
}

void ce_hash_remove(ce_hash* hash, ce_hash_key hash_key)
{
    size_t index = hash_key % hash->entry_count;

    for (ce_hash_entry *hash_entry_prev = NULL, *hash_entry = hash->entries[index];
            NULL != hash_entry; hash_entry_prev = hash_entry, hash_entry = hash_entry->next) {
        if (hash_key == hash_entry->hash_key) {
            if (NULL != hash_entry_prev) {
                hash_entry_prev->next = hash_entry->next;
            } else {
                hash->entries[index] = hash_entry->next;
            }
            ce_hash_entry_del(hash_entry, hash->item_dtor);
            --hash->item_count;
            return;
        }
    }
}

void* ce_hash_find(const ce_hash* hash, ce_hash_key hash_key)
{
    for (ce_hash_entry* hash_entry = hash->entries[hash_key % hash->entry_count];
            NULL != hash_entry; hash_entry = hash_entry->next) {
        if (hash_key == hash_entry->hash_key) {
            return hash_entry->value;
        }
    }
    return NULL;
}

void ce_hash_for_each(ce_hash* hash, void (*func)(void*))
{
    for (size_t i = 0; i < hash->entry_count; ++i) {
        ce_hash_entry_for_each(hash->entries[i], func);
    }
}

void ce_hash_for_each_arg1(ce_hash* hash, void (*func)(void*, void*), void* arg)
{
    for (size_t i = 0; i < hash->entry_count; ++i) {
        ce_hash_entry_for_each_arg1(hash->entries[i], func, arg);
    }
}

void ce_hash_for_each_key(ce_hash* hash, void (*func)(ce_hash_key))
{
    for (size_t i = 0; i < hash->entry_count; ++i) {
        ce_hash_entry_for_each_key(hash->entries[i], func);
    }
}

void ce_hash_iter_first(ce_hash_iter* iter, ce_hash* hash)
{
    iter->hash = hash;

    if (0 != hash->item_count && 0 != hash->entry_count) {
        iter->at_end = false;
        iter->index = 0;
        iter->entry = hash->entries[0];

        if (NULL != iter->entry) {
            iter->value = iter->entry->value;
        } else {
            ce_hash_iter_next(iter);
        }
    } else {
        iter->at_end = true;
    }
}

void ce_hash_iter_next(ce_hash_iter* iter)
{
    while (!iter->at_end) {
        if (NULL != iter->entry) {
            iter->entry = iter->entry->next;
        } else {
            if (++iter->index < iter->hash->entry_count) {
                iter->entry = iter->hash->entries[iter->index];
            } else {
                iter->at_end = true;
            }
        }

        if (NULL != iter->entry) {
            iter->value = iter->entry->value;
            break;
        }
    }
}
