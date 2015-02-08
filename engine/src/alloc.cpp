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

/*
 * Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design Patterns Applied"
 * Copyright (C) 2001 Addison-Wesley
 *
 * The Loki Library
 * Copyright (C) 2001 by Andrei Alexandrescu
 */

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <climits>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "math.hpp"
#include "alloc.hpp"

namespace cursedearth
{
    enum {
        CE_ALLOC_PAGE_SIZE = 4096,
        CE_ALLOC_MAX_SMALL_OBJECT_SIZE = 256,
        CE_ALLOC_OBJECT_ALIGNMENT = 4,
    };

    typedef struct {
#ifdef _WIN32
        CRITICAL_SECTION handle;
#else
        pthread_mutex_t handle;
#endif
    } ce_alloc_mutex;

    typedef struct {
        unsigned char* data;
        unsigned char next_block;
        unsigned char block_count;
    } ce_alloc_chunk;

    typedef struct {
        size_t block_size;
        size_t block_count;
        size_t chunk_count;
        size_t chunk_capacity;
        ce_alloc_mutex mutex;
        ce_alloc_chunk* chunks;
        ce_alloc_chunk* alloc_chunk;
        ce_alloc_chunk* dealloc_chunk;
    } ce_alloc_portion;

    struct {
        bool inited;
        size_t portion_count;
        ce_alloc_portion* portions;
    } ce_alloc_context;

    inline void ce_alloc_mutex_init(ce_alloc_mutex* mutex)
    {
#ifdef _WIN32
        InitializeCriticalSection(&mutex->handle);
#else
        pthread_mutex_init(&mutex->handle, NULL);
#endif
    }

    inline void ce_alloc_mutex_clean(ce_alloc_mutex* mutex)
    {
#ifdef _WIN32
        DeleteCriticalSection(&mutex->handle);
#else
        pthread_mutex_destroy(&mutex->handle);
#endif
    }

    inline void ce_alloc_mutex_lock(ce_alloc_mutex* mutex)
    {
#ifdef _WIN32
        EnterCriticalSection(&mutex->handle);
#else
        pthread_mutex_lock(&mutex->handle);
#endif
    }

    inline void ce_alloc_mutex_unlock(ce_alloc_mutex* mutex)
    {
#ifdef _WIN32
        LeaveCriticalSection(&mutex->handle);
#else
        pthread_mutex_unlock(&mutex->handle);
#endif
    }

    void ce_alloc_chunk_init(ce_alloc_chunk* chunk, size_t block_size, unsigned char block_count)
    {
        chunk->data = (unsigned char*)malloc(block_size * block_count);
        chunk->next_block = 0;
        chunk->block_count = block_count;

        for (unsigned char i = 0, *p = chunk->data; i < block_count; p += block_size) {
            *p = ++i;
        }
    }

    void ce_alloc_chunk_clean(ce_alloc_chunk* chunk)
    {
        if (NULL != chunk) {
            free(chunk->data);
        }
    }

    inline bool ce_alloc_chunk_has_block(const ce_alloc_chunk* chunk, void* ptr, size_t chunk_size)
    {
        unsigned char* p = (unsigned char*)ptr;
        return chunk->data <= p && p < chunk->data + chunk_size;
    }

    void* ce_alloc_chunk_alloc(ce_alloc_chunk* chunk, size_t block_size)
    {
        if (0 == chunk->block_count) {
            return NULL;
        }

        unsigned char* p = chunk->data + block_size * chunk->next_block;
        chunk->next_block = *p;

        --chunk->block_count;

        return p;
    }

    void ce_alloc_chunk_free(ce_alloc_chunk* chunk, void* ptr, size_t block_size)
    {
        unsigned char* p = (unsigned char*)ptr;

        *p = chunk->next_block;
        chunk->next_block = (p - chunk->data) / block_size;

        ++chunk->block_count;
    }

    void ce_alloc_portion_init(ce_alloc_portion* portion, size_t block_size)
    {
        portion->block_size = block_size;
        portion->block_count = clamp(CE_ALLOC_PAGE_SIZE / block_size, CHAR_BIT, UCHAR_MAX);
        portion->chunk_count = 0;
        portion->chunk_capacity = 16;
        portion->chunks = (ce_alloc_chunk*)malloc(sizeof(ce_alloc_chunk) * portion->chunk_capacity);
        portion->alloc_chunk = NULL;
        portion->dealloc_chunk = NULL;
        ce_alloc_mutex_init(&portion->mutex);
    }

    void ce_alloc_portion_clean(ce_alloc_portion* portion)
    {
        if (NULL != portion) {
            for (size_t i = 0; i < portion->chunk_count; ++i) {
                assert(portion->chunks[i].block_count == portion->block_count && "memory leak detected");
                ce_alloc_chunk_clean(portion->chunks + i);
            }
            ce_alloc_mutex_clean(&portion->mutex);
            free(portion->chunks);
        }
    }

    bool ce_alloc_portion_has_block(const ce_alloc_portion* portion, void* ptr)
    {
        const size_t chunk_size = portion->block_size * portion->block_count;
        for (size_t i = 0; i < portion->chunk_count; ++i) {
            if (ce_alloc_chunk_has_block(portion->chunks + i, ptr, chunk_size)) {
                return true;
            }
        }
        return false;
    }

    void ce_alloc_portion_ensure_alloc_chunk(ce_alloc_portion* portion)
    {
        if (NULL != portion->alloc_chunk && 0 != portion->alloc_chunk->block_count) {
            return;
        }

        for (size_t i = 0; i < portion->chunk_count; ++i) {
            ce_alloc_chunk* chunk = portion->chunks + i;
            if (0 != chunk->block_count) {
                portion->alloc_chunk = chunk;
                return;
            }
        }

        if (portion->chunk_count == portion->chunk_capacity) {
            portion->chunk_capacity <<= 1;
            portion->chunks = (ce_alloc_chunk*)realloc(portion->chunks, sizeof(ce_alloc_chunk) * portion->chunk_capacity);
            portion->alloc_chunk = NULL;
            portion->dealloc_chunk = NULL;
        }

        portion->alloc_chunk = portion->chunks + portion->chunk_count;
        ce_alloc_chunk_init(portion->alloc_chunk, portion->block_size, portion->block_count);

        ++portion->chunk_count;
    }

    void ce_alloc_portion_ensure_dealloc_chunk(ce_alloc_portion* portion, void* ptr)
    {
        const size_t chunk_size = portion->block_size * portion->block_count;
        const ce_alloc_chunk* const lo_bound = portion->chunks;
        const ce_alloc_chunk* const hi_bound = portion->chunks + portion->chunk_count;
        ce_alloc_chunk* lo = NULL != portion->dealloc_chunk ? portion->dealloc_chunk : portion->chunks;
        ce_alloc_chunk* hi = lo + 1;

        assert(lo_bound <= lo && hi <= hi_bound);

        if (hi_bound == hi) {
            hi = NULL;
        }

        for (;;) {
            if (NULL != lo) {
                if (ce_alloc_chunk_has_block(lo, ptr, chunk_size)) {
                    portion->dealloc_chunk = lo;
                    return;
                }
                if (lo_bound == lo) {
                    lo = NULL;
                    if (NULL == hi) {
                        break;
                    }
                } else {
                    --lo;
                }
            }

            if (NULL != hi) {
                if (ce_alloc_chunk_has_block(hi, ptr, chunk_size)) {
                    portion->dealloc_chunk = hi;
                    return;
                }
                if (hi_bound == ++hi) {
                    hi = NULL;
                    if (NULL == lo) {
                        break;
                    }
                }
            }
        }

        assert(false);
    }

    void* ce_alloc_portion_alloc(ce_alloc_portion* portion)
    {
        ce_alloc_mutex_lock(&portion->mutex);
        ce_alloc_portion_ensure_alloc_chunk(portion);
        void* ptr = ce_alloc_chunk_alloc(portion->alloc_chunk, portion->block_size);
        ce_alloc_mutex_unlock(&portion->mutex);
        return ptr;
    }

    void ce_alloc_portion_free(ce_alloc_portion* portion, void* ptr)
    {
        ce_alloc_mutex_lock(&portion->mutex);
        ce_alloc_portion_ensure_dealloc_chunk(portion, ptr);
        ce_alloc_chunk_free(portion->dealloc_chunk, ptr, portion->block_size);
        ce_alloc_mutex_unlock(&portion->mutex);
    }

    // calculates offset into array where an element of size is located
    inline size_t ce_alloc_get_offset(size_t size)
    {
        return (size + CE_ALLOC_OBJECT_ALIGNMENT - 1) / CE_ALLOC_OBJECT_ALIGNMENT;
    }

    void ce_alloc_term(void)
    {
        if (ce_alloc_context.inited) {
            if (NULL != ce_alloc_context.portions) {
                for (size_t i = 0; i < ce_alloc_context.portion_count; ++i) {
                    ce_alloc_portion_clean(ce_alloc_context.portions + i);
                }
                free(ce_alloc_context.portions);
            }
            ce_alloc_context.inited = false;
        }
    }

    void ce_alloc_init(void)
    {
        if (!ce_alloc_context.inited) {
            ce_alloc_context.inited = true;
            atexit(ce_alloc_term);
            ce_alloc_context.portion_count = ce_alloc_get_offset(CE_ALLOC_MAX_SMALL_OBJECT_SIZE);
            ce_alloc_context.portions = (ce_alloc_portion*)malloc(sizeof(ce_alloc_portion) * ce_alloc_context.portion_count);
            for (size_t i = 0; i < ce_alloc_context.portion_count; ++i) {
                ce_alloc_portion_init(ce_alloc_context.portions + i, (i + 1) * CE_ALLOC_OBJECT_ALIGNMENT);
            }
        }
    }

    void* ce_alloc(size_t size)
    {
        assert(ce_alloc_context.inited && "alloc subsystem has not yet been inited");
        size = std::max<size_t>(1, size);
        return size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE ? malloc(size) : ce_alloc_portion_alloc(ce_alloc_context.portions + ce_alloc_get_offset(size) - 1);
    }

    void* ce_alloc_zero(size_t size)
    {
        return memset(ce_alloc(size), 0, size);
    }

    void* ce_realloc(void* ptr, size_t size, size_t new_size)
    {
        void* new_ptr = ce_alloc(new_size);
        memcpy(new_ptr, ptr, std::min(size, new_size));
        ce_free(ptr, size);
        return new_ptr;
    }

    void ce_free(void* ptr, size_t size)
    {
        assert(ce_alloc_context.inited && "alloc subsystem has not yet been inited");
        size = std::max<size_t>(1, size);
        if (size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE) {
            free(ptr);
        } else if (NULL != ptr) {
            ce_alloc_portion_free(ce_alloc_context.portions + ce_alloc_get_offset(size) - 1, ptr);
        }
    }

    void ce_free_slow(void* ptr)
    {
        assert(ce_alloc_context.inited && "alloc subsystem has not yet been inited");
        size_t i;
        for (i = 0; i < ce_alloc_context.portion_count; ++i) {
            if (ce_alloc_portion_has_block(ce_alloc_context.portions + i, ptr)) {
                break;
            }
        }
        if (i == ce_alloc_context.portion_count) {
            free(ptr);
        } else if (NULL != ptr) {
            ce_alloc_portion_free(ce_alloc_context.portions + i, ptr);
        }
    }
}
