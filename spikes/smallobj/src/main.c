#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

#include "memory.h"

typedef struct {
	size_t array_size;
	int* array;
	size_t name_size;
	char* name;
} test;

static test* test_open(size_t array_size, size_t name_size)
{
	test* t = memory_alloc(sizeof(test));
	t->array_size = array_size;
	t->array = memory_alloc(sizeof(int) * array_size);
	for (size_t i = 0; i < array_size; ++i) {
		t->array[i] = rand();
	}
	t->name_size = name_size;
	t->name = memory_alloc(name_size);
	for (size_t i = 0; i < name_size; ++i) {
		t->name[i] = 'a' + rand() % 10;
	}
	t->name[name_size - 1] = '\0';
	return t;
}

static void test_close(test* t)
{
	memory_free(t->name, t->name_size);
	memory_free(t->array, sizeof(int) * t->array_size);
	memory_free(t, sizeof(test));
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	if (!memory_open()) {
		printf("Could not open memory module\n");
	}

	assert(2 == argc);

	const size_t block = 10;
	const size_t count = atoi(argv[1]);

	test* p[count][block];

	for (size_t i = 0; i < count; ++i) {
		for (size_t j = 0; j < block; ++j) {
			p[i][j] = test_open(rand() % 25 + 1, rand() % 80 + 1);
		}
	}

	for (size_t i = 0; i < 1000; ++i) {
		size_t idx1 = rand() % count;
		size_t idx2 = rand() % count;
		for (size_t j = 0; j < block; ++j) {
			test* t = p[idx1][j];
			p[idx1][j] = p[idx2][j];
			p[idx2][j] = t;
		}
	}

	for (size_t i = 0; i < count; ++i) {
		for (size_t j = 0; j < block; ++j) {
			test_close(p[i][j]);
		}
	}

	memory_close();
	return 0;
}
