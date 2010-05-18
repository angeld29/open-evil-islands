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
 *
 *  1. Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
 *     Patterns Applied". Copyright (C) 2001 Addison-Wesley.
 *
 *  2. The Loki Library (C) 2001 by Andrei Alexandrescu.
*/

/*
 *  Notes:
 *
 *  1. Minimum error checking. Disable smallobj (MAX_SMALL_OBJECT_SIZE = 0)
 *     and use, for example, valgrind on Linux to perform full error checking.
 *
 *  2. Alloc have no external dependencies, so we must
 *     re-implement synchronization stuff here.
 *
 *  3. There are no comments! If this subject is interesting
 *     for you just read the book or see the library.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "celib.h"
#include "cealloc.h"

static const size_t CE_ALLOC_PAGE_SIZE = 4096;
static const size_t CE_ALLOC_MAX_SMALL_OBJECT_SIZE = 256;
static const size_t CE_ALLOC_OBJECT_ALIGNMENT = 4;
static const size_t CE_ALLOC_ONE = 1;

typedef struct {
#ifdef _WIN32
	CRITICAL_SECTION cs;
#else
	pthread_mutex_t mutex;
#endif
} ce_alloc_mutex;

static void ce_alloc_mutex_init(ce_alloc_mutex* mutex)
{
#ifdef _WIN32
	InitializeCriticalSection(&mutex->cs);
#else
	pthread_mutex_init(&mutex->mutex, NULL);
#endif
}

static void ce_alloc_mutex_clean(ce_alloc_mutex* mutex)
{
#ifdef _WIN32
	DeleteCriticalSection(&mutex->cs);
#else
	pthread_mutex_destroy(&mutex->mutex);
#endif
}

static void ce_alloc_mutex_lock(ce_alloc_mutex* mutex)
{
#ifdef _WIN32
	EnterCriticalSection(&mutex->cs);
#else
	pthread_mutex_lock(&mutex->mutex);
#endif
}

static void ce_alloc_mutex_unlock(ce_alloc_mutex* mutex)
{
#ifdef _WIN32
	LeaveCriticalSection(&mutex->cs);
#else
	pthread_mutex_unlock(&mutex->mutex);
#endif
}

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
	ce_alloc_chunk* chunks;
	ce_alloc_chunk* alloc_chunk;
	ce_alloc_chunk* dealloc_chunk;
	ce_alloc_mutex mutex;
} ce_alloc_portion;

static struct {
	bool inited;
	size_t portion_count;
	ce_alloc_portion* portions;
#ifndef NDEBUG
	size_t smallobj_allocated;
	size_t smallobj_max_allocated;
	size_t smallobj_overhead;
	size_t system_allocated;
	size_t system_max_allocated;
	ce_alloc_mutex mutex;
#endif
} ce_alloc_context;

static void ce_alloc_chunk_init(ce_alloc_chunk* chunk, size_t block_size, unsigned char block_count)
{
	chunk->data = malloc(block_size * block_count);
    chunk->next_block = 0;
    chunk->block_count = block_count;

	for (unsigned char i = 0, *p = chunk->data; i < block_count; p += block_size) {
		*p = ++i;
	}
}

static void ce_alloc_chunk_clean(ce_alloc_chunk* chunk)
{
	if (NULL != chunk) {
		free(chunk->data);
	}
}

static void* ce_alloc_chunk_alloc(ce_alloc_chunk* chunk, size_t block_size)
{
	if (0 == chunk->block_count) {
		return NULL;
	}

	unsigned char* p = chunk->data + block_size * chunk->next_block;
	chunk->next_block = *p;

	--chunk->block_count;

	return p;
}

static void ce_alloc_chunk_free(ce_alloc_chunk* chunk, void* ptr, size_t block_size)
{
	unsigned char* p = ptr;

	*p = chunk->next_block;
	chunk->next_block = (p - chunk->data) / block_size;

    ++chunk->block_count;
}

static void ce_alloc_portion_init(ce_alloc_portion* portion, size_t block_size)
{
	portion->block_size = block_size;
	portion->block_count = ce_sclamp(CE_ALLOC_PAGE_SIZE / block_size, CHAR_BIT, UCHAR_MAX);
	portion->chunk_count = 0;
	portion->chunk_capacity = 16;
	portion->chunks = malloc(sizeof(ce_alloc_chunk) * portion->chunk_capacity);
	portion->alloc_chunk = NULL;
	portion->dealloc_chunk = NULL;
	ce_alloc_mutex_init(&portion->mutex);
}

static void ce_alloc_portion_clean(ce_alloc_portion* portion)
{
	if (NULL != portion) {
		ce_alloc_mutex_clean(&portion->mutex);
		for (size_t i = 0; i < portion->chunk_count; ++i) {
			assert(portion->chunks[i].block_count ==
					portion->block_count && "memory leak detected");
			ce_alloc_chunk_clean(portion->chunks + i);
		}
		free(portion->chunks);
	}
}

static void ce_alloc_portion_ensure_alloc_chunk(ce_alloc_portion* portion)
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
		portion->chunks = realloc(portion->chunks, sizeof(ce_alloc_chunk) *
													portion->chunk_capacity);
		portion->alloc_chunk = NULL;
		portion->dealloc_chunk = NULL;
	}

	portion->alloc_chunk = portion->chunks + portion->chunk_count;
	ce_alloc_chunk_init(portion->alloc_chunk, portion->block_size, portion->block_count);

	++portion->chunk_count;
}

static void ce_alloc_portion_ensure_dealloc_chunk(ce_alloc_portion* portion, void* ptr)
{
	unsigned char* p = ptr;
	const size_t chunk_size = portion->block_size * portion->block_count;
	const ce_alloc_chunk* const lo_bound = portion->chunks;
	const ce_alloc_chunk* const hi_bound = portion->chunks + portion->chunk_count;
	ce_alloc_chunk* lo = NULL != portion->dealloc_chunk ?
								portion->dealloc_chunk : portion->chunks;
	ce_alloc_chunk* hi = lo + CE_ALLOC_ONE;

	assert(lo_bound <= lo && hi <= hi_bound);

	if (hi_bound == hi) {
		hi = NULL;
	}

	for (;;) {
		if (NULL != lo) {
			if (lo->data <= p && p < lo->data + chunk_size) {
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
			if (hi->data <= p && p < hi->data + chunk_size) {
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

static void* ce_alloc_portion_alloc(ce_alloc_portion* portion)
{
	ce_alloc_mutex_lock(&portion->mutex);
	ce_alloc_portion_ensure_alloc_chunk(portion);
	void* ptr = ce_alloc_chunk_alloc(portion->alloc_chunk, portion->block_size);
	ce_alloc_mutex_unlock(&portion->mutex);
	return ptr;
}

static void ce_alloc_portion_free(ce_alloc_portion* portion, void* ptr)
{
	ce_alloc_mutex_lock(&portion->mutex);
	ce_alloc_portion_ensure_dealloc_chunk(portion, ptr);
	ce_alloc_chunk_free(portion->dealloc_chunk, ptr, portion->block_size);
	ce_alloc_mutex_unlock(&portion->mutex);
}

// calculates offset into array where an element of size is located
static inline size_t get_offset(size_t size)
{
	return (size + CE_ALLOC_OBJECT_ALIGNMENT - CE_ALLOC_ONE) / CE_ALLOC_OBJECT_ALIGNMENT;
}

bool ce_alloc_init(void)
{
	assert(!ce_alloc_context.inited && "the alloc subsystem has already been inited");

	ce_alloc_context.portion_count = get_offset(CE_ALLOC_MAX_SMALL_OBJECT_SIZE);
	ce_alloc_context.portions = malloc(sizeof(ce_alloc_portion) *
										ce_alloc_context.portion_count);

	for (size_t i = 0; i < ce_alloc_context.portion_count; ++i) {
		ce_alloc_portion_init(ce_alloc_context.portions + i,
			(i + CE_ALLOC_ONE) * CE_ALLOC_OBJECT_ALIGNMENT);
	}

#ifndef NDEBUG
	ce_alloc_mutex_init(&ce_alloc_context.mutex);
#endif

	return ce_alloc_context.inited = true;
}

void ce_alloc_term(void)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");
	ce_alloc_context.inited = false;

#ifndef NDEBUG
	ce_alloc_mutex_clean(&ce_alloc_context.mutex);
#endif

	if (NULL != ce_alloc_context.portions) {
		for (size_t i = 0; i < ce_alloc_context.portion_count; ++i) {
			ce_alloc_portion_clean(ce_alloc_context.portions + i);
		}

		free(ce_alloc_context.portions);
		ce_alloc_context.portions = NULL;
	}
}

void* ce_alloc(size_t size)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");

	size = ce_smax(CE_ALLOC_ONE, size);

#ifndef NDEBUG
	// needs to lock mutex here to avoid race conditions
	// in portion_alloc and statistics
	ce_alloc_mutex_lock(&ce_alloc_context.mutex);
#endif

	void* ptr = size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE ? malloc(size) :
		ce_alloc_portion_alloc(ce_alloc_context.portions +
								get_offset(size) - CE_ALLOC_ONE);

#ifndef NDEBUG
	ce_alloc_context.smallobj_allocated += size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE ? 0 : size;
	ce_alloc_context.smallobj_max_allocated =
		ce_smax(ce_alloc_context.smallobj_allocated,
				ce_alloc_context.smallobj_max_allocated);
	ce_alloc_context.system_allocated += size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE ? size : 0;
	ce_alloc_context.system_max_allocated =
		ce_smax(ce_alloc_context.system_allocated,
				ce_alloc_context.system_max_allocated);

	ce_alloc_context.smallobj_overhead = 0;
	for (size_t i = 0; i < ce_alloc_context.portion_count; ++i) {
		ce_alloc_portion* portion = ce_alloc_context.portions + i;
		ce_alloc_context.smallobj_overhead +=
			portion->block_size * portion->block_count *
			(portion->chunk_capacity - portion->chunk_count);
		for (size_t j = 0; j < portion->chunk_count; ++j) {
			ce_alloc_chunk* chunk = portion->chunks + j;
			ce_alloc_context.smallobj_overhead +=
				portion->block_size * chunk->block_count;
		}
	}
	ce_alloc_mutex_unlock(&ce_alloc_context.mutex);
#endif

	return ptr;
}

void* ce_alloc_zero(size_t size)
{
	return memset(ce_alloc(size), 0, size);
}

void ce_free(void* ptr, size_t size)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");

	size = ce_smax(CE_ALLOC_ONE, size);

#ifndef NDEBUG
	if (NULL != ptr) {
		ce_alloc_mutex_lock(&ce_alloc_context.mutex);
		ce_alloc_context.smallobj_allocated -=
			size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE ? 0 : size;
		ce_alloc_context.system_allocated -=
			size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE ? size : 0;
		ce_alloc_mutex_unlock(&ce_alloc_context.mutex);
	}
#endif

	if (size > CE_ALLOC_MAX_SMALL_OBJECT_SIZE) {
		free(ptr);
	} else if (NULL != ptr) {
		ce_alloc_portion_free(ce_alloc_context.portions +
								get_offset(size) - CE_ALLOC_ONE, ptr);
	}
}

#ifndef NDEBUG
size_t ce_alloc_get_smallobj_allocated(void)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");
	return ce_alloc_context.smallobj_allocated;
}

size_t ce_alloc_get_smallobj_max_allocated(void)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");
	return ce_alloc_context.smallobj_max_allocated;
}

size_t ce_alloc_get_smallobj_overhead(void)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");
	return ce_alloc_context.smallobj_overhead;
}

size_t ce_alloc_get_system_allocated(void)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");
	return ce_alloc_context.system_allocated;
}

size_t ce_alloc_get_system_max_allocated(void)
{
	assert(ce_alloc_context.inited && "the alloc subsystem has not yet been inited");
	return ce_alloc_context.system_max_allocated;
}
#endif
