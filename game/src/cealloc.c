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

/*
 *  Based on:
 *  1. Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
 *     Patterns Applied". Copyright (C) 2001. Addison-Wesley.
 *  2. The Loki Library (C) 2001 by Andrei Alexandrescu.
*/

/*
 *  Notes:
 *  1. Minimum error checking. Disable smallobj and use malloc/free
 *     with valgrind on Linux to perform full error checking.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"

static const size_t PAGE_SIZE = 4096;
static const size_t MAX_SMALL_OBJECT_SIZE = 256;
static const size_t OBJECT_ALIGNMENT = 4;

typedef struct {
	unsigned char* data;
	unsigned char next_block;
	unsigned char block_count;
} chunk;

typedef struct {
	size_t block_size;
	size_t block_count;
	size_t chunk_count;
	size_t chunk_capacity;
	chunk* chunks;
	chunk* alloc_chunk;
	chunk* dealloc_chunk;
} portion;

static struct {
	bool inited;
	size_t count;
	portion* pool;
	size_t smallobj_allocated;
	size_t smallobj_max_allocated;
	size_t smallobj_overhead;
	size_t system_allocated;
	size_t system_max_allocated;
} ce_alloc_inst;

static void chunk_init(chunk* cnk, size_t block_size, unsigned char block_count)
{
	cnk->data = malloc(block_size * block_count);
    cnk->next_block = 0;
    cnk->block_count = block_count;

	for (unsigned char i = 0, *p = cnk->data; i < block_count; p += block_size) {
		*p = ++i;
	}
}

static void chunk_clean(chunk* cnk)
{
	if (NULL != cnk) {
		free(cnk->data);
	}
}

static void* chunk_alloc(chunk* cnk, size_t block_size)
{
	if (0 == cnk->block_count) {
		return NULL;
	}

	unsigned char* p = cnk->data + block_size * cnk->next_block;
	cnk->next_block = *p;

	--cnk->block_count;

	return p;
}

static void chunk_free(chunk* cnk, void* ptr, size_t block_size)
{
	unsigned char* p = ptr;

	*p = cnk->next_block;
	cnk->next_block = (p - cnk->data) / block_size;

    ++cnk->block_count;
}

static void portion_init(portion* por, size_t block_size, size_t page_size)
{
	por->block_size = block_size;
	por->block_count = ce_sclamp(page_size / block_size, CHAR_BIT, UCHAR_MAX);
	por->chunk_count = 0;
	por->chunk_capacity = 16;
	por->chunks = malloc(sizeof(chunk) * por->chunk_capacity);
	por->alloc_chunk = NULL;
	por->dealloc_chunk = NULL;
}

static void portion_clean(portion* por)
{
	if (NULL != por) {
		if (NULL != por->chunks) {
			for (size_t i = 0; i < por->chunk_count; ++i) {
				assert(por->chunks[i].block_count ==
						por->block_count && "Memory leak detected");
				chunk_clean(por->chunks + i);
			}
			free(por->chunks);
		}
	}
}

static bool portion_ensure_alloc_chunk(portion* por)
{
	if (NULL != por->alloc_chunk && 0 != por->alloc_chunk->block_count) {
		return true;
	}

	for (size_t i = 0; i < por->chunk_count; ++i) {
		chunk* cnk = por->chunks + i;
		if (0 != cnk->block_count) {
			por->alloc_chunk = cnk;
			return true;
		}
	}

	if (por->chunk_count == por->chunk_capacity) {
		size_t chunk_capacity = 2 * por->chunk_capacity;
		chunk* chunks = realloc(por->chunks, sizeof(chunk) * chunk_capacity);
		if (NULL == chunks) {
			return false;
		}
		por->chunk_capacity = chunk_capacity;
		por->chunks = chunks;
		por->alloc_chunk = NULL;
		por->dealloc_chunk = NULL;
	}

	chunk* alloc_chunk = por->chunks + por->chunk_count;

	chunk_init(alloc_chunk, por->block_size, por->block_count);

	por->alloc_chunk = alloc_chunk;
	++por->chunk_count;

	return true;
}

static void portion_ensure_dealloc_chunk(portion* por, void* ptr)
{
	unsigned char* p = ptr;
	const size_t chunk_size = por->block_size * por->block_count;
	const chunk* const lo_bound = por->chunks;
	const chunk* const hi_bound = por->chunks + por->chunk_count;
	chunk* lo = NULL != por->dealloc_chunk ? por->dealloc_chunk : por->chunks;
	chunk* hi = lo + 1;

	assert(lo_bound <= lo && hi <= hi_bound);

	if (hi_bound == hi) {
		hi = NULL;
	}

	for (;;) {
		if (NULL != lo) {
			if (lo->data <= p && p < lo->data + chunk_size) {
				por->dealloc_chunk = lo;
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
			if (hi->data <= p && p < hi->data + chunk_size) {
				por->dealloc_chunk = hi;
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

static void* portion_alloc(portion* por)
{
	return portion_ensure_alloc_chunk(por) ?
		chunk_alloc(por->alloc_chunk, por->block_size) : NULL;
}

static void portion_free(portion* por, void* ptr)
{
	portion_ensure_dealloc_chunk(por, ptr);
	chunk_free(por->dealloc_chunk, ptr, por->block_size);
}

/// Calculates offset into array where an element of size is located.
static size_t get_offset(size_t size, size_t alignment)
{
	return (size + alignment - 1) / alignment;
}

bool ce_alloc_init(void)
{
	assert(!ce_alloc_inst.inited && "The alloc subsystem has already been inited");
	ce_alloc_inst.inited = true;

	ce_alloc_inst.count = get_offset(MAX_SMALL_OBJECT_SIZE, OBJECT_ALIGNMENT);
	if (NULL == (ce_alloc_inst.pool = calloc(ce_alloc_inst.count,
												sizeof(portion)))) {
		return false;
	}

	for (size_t i = 0; i < ce_alloc_inst.count; ++i) {
		portion_init(ce_alloc_inst.pool + i,
					(i + 1) * OBJECT_ALIGNMENT, PAGE_SIZE);
	}

	return true;
}

void ce_alloc_term(void)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");
	ce_alloc_inst.inited = false;

	if (NULL != ce_alloc_inst.pool) {
		for (size_t i = 0; i < ce_alloc_inst.count; ++i) {
			portion_clean(ce_alloc_inst.pool + i);
		}

		free(ce_alloc_inst.pool);
		ce_alloc_inst.pool = NULL;
	}
}

#ifndef CE_NDEBUG_MEMORY
static void ce_alloc_update_alloc_stat(size_t size)
{
	ce_alloc_inst.smallobj_allocated += size > MAX_SMALL_OBJECT_SIZE ? 0 : size;
	ce_alloc_inst.smallobj_max_allocated =
		ce_smax(ce_alloc_inst.smallobj_allocated,
				ce_alloc_inst.smallobj_max_allocated);

	ce_alloc_inst.system_allocated += size > MAX_SMALL_OBJECT_SIZE ? size : 0;
	ce_alloc_inst.system_max_allocated =
		ce_smax(ce_alloc_inst.system_allocated,
				ce_alloc_inst.system_max_allocated);
}

static void ce_alloc_update_smallobj_overhead(void)
{
	ce_alloc_inst.smallobj_overhead = 0;
	for (size_t i = 0; i < ce_alloc_inst.count; ++i) {
		portion* por = ce_alloc_inst.pool + i;
		ce_alloc_inst.smallobj_overhead +=
			por->block_size * por->block_count *
			(por->chunk_capacity - por->chunk_count);
		for (size_t j = 0; j < por->chunk_count; ++j) {
			chunk* cnk = por->chunks + j;
			ce_alloc_inst.smallobj_overhead +=
				por->block_size * cnk->block_count;
		}
	}
}
#endif /* !CE_NDEBUG_MEMORY */

void* ce_alloc(size_t size)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");

	void* ptr = size > MAX_SMALL_OBJECT_SIZE ? malloc(size) :
		portion_alloc(ce_alloc_inst.pool +
					get_offset(ce_smax(1, size), OBJECT_ALIGNMENT) - 1);

#ifndef CE_NDEBUG_MEMORY
	ce_alloc_update_alloc_stat(ce_smax(1, size));
	ce_alloc_update_smallobj_overhead();
#endif

	return ptr;
}

void* ce_alloc_zero(size_t size)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");

#ifndef CE_NDEBUG_MEMORY
	ce_alloc_update_alloc_stat(ce_smax(1, size));
#endif

	if (size > MAX_SMALL_OBJECT_SIZE) {
		return calloc(1, size);
	}

	size = ce_smax(1, size);
	void* ptr = portion_alloc(ce_alloc_inst.pool +
							get_offset(size, OBJECT_ALIGNMENT) - 1);

#ifndef CE_NDEBUG_MEMORY
	ce_alloc_update_smallobj_overhead();
#endif

	return NULL != ptr ? memset(ptr, 0, size) : NULL;
}

void ce_free(void* ptr, size_t size)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");

#ifndef CE_NDEBUG_MEMORY
	if (NULL != ptr) {
		ce_alloc_inst.smallobj_allocated -=
			size > MAX_SMALL_OBJECT_SIZE ? 0 : size;
		ce_alloc_inst.system_allocated -=
			size > MAX_SMALL_OBJECT_SIZE ? size : 0;
	}
#endif

	if (size > MAX_SMALL_OBJECT_SIZE) {
		free(ptr);
	} else if (NULL != ptr) {
		portion_free(ce_alloc_inst.pool +
					get_offset(ce_smax(1, size), OBJECT_ALIGNMENT) - 1, ptr);
	}
}

size_t ce_alloc_get_smallobj_allocated(void)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");
	return ce_alloc_inst.smallobj_allocated;
}

size_t ce_alloc_get_smallobj_max_allocated(void)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");
	return ce_alloc_inst.smallobj_max_allocated;
}

size_t ce_alloc_get_smallobj_overhead(void)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");
	return ce_alloc_inst.smallobj_overhead;
}

size_t ce_alloc_get_system_allocated(void)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");
	return ce_alloc_inst.system_allocated;
}

size_t ce_alloc_get_system_max_allocated(void)
{
	assert(ce_alloc_inst.inited && "The alloc subsystem has not yet been inited");
	return ce_alloc_inst.system_max_allocated;
}
