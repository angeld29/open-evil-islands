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
	size_t count;
	portion* pool;
} smallobj;

static bool chunk_is_filled(const chunk* cnk)
{
	return 0 == cnk->block_count;
}

static bool chunk_has_block(void* ptr, size_t chunk_size, chunk* cnk)
{
	unsigned char* p = ptr;
	return cnk->data <= p && p < cnk->data + chunk_size;
}

static bool chunk_init(size_t block_size, unsigned char block_count, chunk* cnk)
{
    if (NULL == (cnk->data = malloc(block_size * block_count))) {
		return false;
	}

    cnk->next_block = 0;
    cnk->block_count = block_count;

	for (unsigned char i = 0, *p = cnk->data; i < block_count; p += block_size) {
		*p = ++i;
	}

	return true;
}

static void chunk_clean(chunk* cnk)
{
	if (NULL == cnk) {
		return;
	}

	free(cnk->data);
}

static void* chunk_alloc(size_t block_size, chunk* cnk)
{
	if (0 == cnk->block_count) {
		return NULL;
	}

	unsigned char* p = cnk->data + block_size * cnk->next_block;
	cnk->next_block = *p;

	--cnk->block_count;

	return p;
}

static void chunk_free(void* ptr, size_t block_size, chunk* cnk)
{
	unsigned char* p = ptr;

	*p = cnk->next_block;
	cnk->next_block = (p - cnk->data) / block_size;

    ++cnk->block_count;
}

static bool portion_init(size_t block_size, size_t page_size, portion* por)
{
	por->block_size = block_size;
	por->block_count = sclamp(page_size / block_size, CHAR_BIT, UCHAR_MAX);

	por->chunk_count = 0;
	por->chunk_capacity = 16;

	if (NULL == (por->chunks = malloc(sizeof(chunk) * por->chunk_capacity))) {
		return false;
	}

	por->alloc_chunk = NULL;
	por->dealloc_chunk = NULL;

	return true;
}

static void portion_clean(portion* por)
{
	if (NULL == por) {
		return;
	}

	if (NULL != por->chunks) {
		for (size_t i = 0; i < por->chunk_count; ++i) {
			assert(por->chunks[i].block_count ==
					por->block_count && "Memory leak detected");
			chunk_clean(por->chunks + i);
		}
	}

	free(por->chunks);
}

static bool portion_ensure_alloc_chunk(portion* por)
{
	if (NULL != por->alloc_chunk && !chunk_is_filled(por->alloc_chunk)) {
		return true;
	}

	for (size_t i = 0; i < por->chunk_count; ++i) {
		chunk* cnk = por->chunks + i;
		if (!chunk_is_filled(cnk)) {
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

	if (!chunk_init(por->block_size, por->block_count, alloc_chunk)) {
		return false;
	}

	por->alloc_chunk = alloc_chunk;
	++por->chunk_count;

	return true;
}

static void portion_ensure_dealloc_chunk(void* ptr, portion* por)
{
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
			if (chunk_has_block(ptr, chunk_size, lo)) {
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
			if (chunk_has_block(ptr, chunk_size, hi)) {
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
		chunk_alloc(por->block_size, por->alloc_chunk) : NULL;
}

static void portion_free(void* ptr, portion* por)
{
	portion_ensure_dealloc_chunk(ptr, por);
	chunk_free(ptr, por->block_size, por->dealloc_chunk);
}

/// Calculates offset into array where an element of size is located.
static size_t get_offset(size_t size, size_t alignment)
{
	return (size + alignment - 1) / alignment;
}

bool cealloc_open(void)
{
	assert(NULL == smallobj.pool);

	smallobj.count = get_offset(MAX_SMALL_OBJECT_SIZE, OBJECT_ALIGNMENT);
	if (NULL == (smallobj.pool = calloc(smallobj.count, sizeof(portion)))) {
		return false;
	}

	for (size_t i = 0; i < smallobj.count; ++i) {
		if (!portion_init((i + 1) * OBJECT_ALIGNMENT,
				PAGE_SIZE, smallobj.pool + i)) {
			cealloc_close();
			return false;
		}
	}

	return true;
}

void cealloc_close(void)
{
	assert(NULL != smallobj.pool);

	if (NULL != smallobj.pool) {
		for (size_t i = 0; i < smallobj.count; ++i) {
			portion_clean(smallobj.pool + i);
		}
	}

	free(smallobj.pool);
	smallobj.pool = NULL;
}

void* cealloc(size_t size)
{
	assert(NULL != smallobj.pool);

	if (size > MAX_SMALL_OBJECT_SIZE) {
		return malloc(size);
	}

	if (0 == size) {
		size = 1;
	}

	return portion_alloc(smallobj.pool + get_offset(size, OBJECT_ALIGNMENT) - 1);
}

void* cealloczero(size_t size)
{
	void* ptr = cealloc(size);
	return NULL != ptr ? memset(ptr, 0, size) : NULL;
}

void cefree(void* ptr, size_t size)
{
	assert(NULL != smallobj.pool);

	if (size > MAX_SMALL_OBJECT_SIZE) {
		free(ptr);
		return;
	}

	if (NULL == ptr) {
		return;
	}

	if (0 == size) {
		size = 1;
	}

	portion_free(ptr, smallobj.pool + get_offset(size, OBJECT_ALIGNMENT) - 1);
}
