/*
 *  Based on:
 *  1. Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
 *     Patterns Applied". Copyright (C) 2001. Addison-Wesley.
 *  2. The Loki Library (C) 2001 by Andrei Alexandrescu.
*/

#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "smallobj.h"

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
	chunk* free_chunk;
} portion;

static struct {
	size_t count;
	portion* pool;
} smallobj;

static bool chunk_is_filled(const chunk* cnk)
{
	return 0 == cnk->block_count;
}

static bool chunk_has_block(void* p, size_t chunk_size, chunk* cnk)
{
	unsigned char* pc = p;
	return cnk->data <= pc && pc < cnk->data + chunk_size;
}

static bool chunk_init(size_t block_size, unsigned char block_count, chunk* cnk)
{
	assert(block_size > 0);
	assert(block_count > 0);

	const size_t chunk_size = block_size * block_count;

	// Overflow check.
	assert(chunk_size / block_size == block_count);

    if (NULL == (cnk->data = malloc(chunk_size))) {
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

	assert(cnk->next_block * block_size / block_size == cnk->next_block);

	unsigned char* p = cnk->data + cnk->next_block * block_size;
	cnk->next_block = *p;

	--cnk->block_count;

	return p;
}

static void chunk_free(void* p, size_t block_size, chunk* cnk)
{
	unsigned char* pc = p;

	assert(pc >= cnk->data);

	// Alignment check.
    assert(0 == (pc - cnk->data) % block_size);

	unsigned char index = (pc - cnk->data) / block_size;

	// Check if block was already deleted.
	assert(cnk->block_count > 0 ? cnk->next_block != index : true);

	*pc = cnk->next_block;
	cnk->next_block = index;

    // Truncation check.
    assert((pc - cnk->data) / block_size == cnk->next_block);

    ++cnk->block_count;
}

static bool portion_init(size_t block_size, size_t page_size, portion* por)
{
	assert(block_size > 0);
	assert(page_size >= block_size);

	por->block_size = block_size;
	por->block_count = sclamp(page_size / block_size, CHAR_BIT, UCHAR_MAX);

	por->chunk_count = 0;
	por->chunk_capacity = 16;
	if (NULL == (por->chunks = malloc(por->chunk_capacity * sizeof(chunk)))) {
		return false;
	}

	por->alloc_chunk = NULL;
	por->free_chunk = NULL;

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
				por->block_count && "Memleak!");
			chunk_clean(por->chunks + i);
		}
	}

	free(por->chunks);
}

static chunk* portion_find_chunk(void* p, portion* por)
{
	const size_t chunk_size = por->block_size * por->block_count;
	for (size_t i = 0; i < por->chunk_count; ++i) {
		chunk* cnk = por->chunks + i;
		if (chunk_has_block(p, chunk_size, cnk)) {
			return cnk;
		}
	}
	return NULL;
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
		chunk* chunks = realloc(por->chunks, chunk_capacity * sizeof(chunk));
		if (NULL == chunks) {
			return false;
		}
		por->chunk_capacity = chunk_capacity;
		por->chunks = chunks;
	}

	if (!chunk_init(por->block_size, por->block_count,
			por->chunks + por->chunk_count)) {
		return false;
	}

	por->alloc_chunk = por->chunks + por->chunk_count;
	++por->chunk_count;

	return true;
}

static void* portion_alloc(portion* por)
{
	return portion_ensure_alloc_chunk(por) ?
		chunk_alloc(por->block_size, por->alloc_chunk) : NULL;
}

static void portion_free(void* p, chunk* hint, portion* por)
{
	if (NULL == hint) {
		hint = portion_find_chunk(p, por);
	}
	assert(NULL != hint);
	chunk_free(p, por->block_size, hint);
}

/// Calculates offset into array where an element of size is located.
static size_t get_offset(size_t size, size_t alignment)
{
	return (size + alignment - 1) / alignment;
}

bool smallobj_open(void)
{
	assert(NULL == smallobj.pool);

	smallobj.count = get_offset(MAX_SMALL_OBJECT_SIZE, OBJECT_ALIGNMENT);
	if (NULL == (smallobj.pool = calloc(smallobj.count, sizeof(portion)))) {
		return false;
	}

	for (size_t i = 0; i < smallobj.count; ++i) {
		if (!portion_init((i + 1) * OBJECT_ALIGNMENT,
				PAGE_SIZE, smallobj.pool + i)) {
			smallobj_close();
			return false;
		}
	}

	return true;
}

void smallobj_close(void)
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

void* smallobj_alloc(size_t size)
{
	assert(NULL != smallobj.pool);
	assert(size <= MAX_SMALL_OBJECT_SIZE);

	if (0 == size) {
		size = 1;
	}

	const size_t index = get_offset(size, OBJECT_ALIGNMENT) - 1;
	assert(index < smallobj.count);

	portion* por = smallobj.pool + index;
	assert(por->block_size >= size);
	assert(por->block_size < size + OBJECT_ALIGNMENT);

	return portion_alloc(por);
}

void smallobj_free(void* p, size_t size)
{
	assert(NULL != smallobj.pool);
	assert(size <= MAX_SMALL_OBJECT_SIZE);

	if (NULL == p) {
		return;
	}

	if (0 == size) {
		size = 1;
	}

	const size_t index = get_offset(size, OBJECT_ALIGNMENT) - 1;
	assert(index < smallobj.count);

	portion* por = smallobj.pool + index;
	assert(por->block_size >= size);
	assert(por->block_size < size + OBJECT_ALIGNMENT);

	portion_free(p, NULL, por);
}

size_t smallobj_max_size(void)
{
	return MAX_SMALL_OBJECT_SIZE;
}
