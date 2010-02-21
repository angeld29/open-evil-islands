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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "celogging.h"
#include "cealloc.h"
#include "cestring.h"

static void debug_print(FILE* anm)
{
	size_t r;
	uint32_t count;

	r = fread(&count, 4, 1, anm);
	assert(1 == r);
	printf("count: %u\n", count);
	for (int i = 0, n = count; i < n; ++i) {
		float f[4];
		r = fread(&f, 4, 4, anm);
		assert(4 == r);
		printf("%f %f %f %f\n", f[0], f[1], f[2], f[3]);
	}
	printf("\n");

	r = fread(&count, 4, 1, anm);
	assert(1 == r);
	printf("count: %u\n", count);
	for (int i = 0, n = count; i < n; ++i) {
		float f[3];
		r = fread(&f, 4, 3, anm);
		assert(3 == r);
		printf("%f %f %f\n", f[0], f[1], f[2]);
	}
	printf("\n");

	uint32_t unknown;

	r = fread(&unknown, 4, 1, anm);
	assert(1 == r);
	assert(0 == unknown);

	r = fread(&unknown, 4, 1, anm);
	assert(1 == r);
	assert(0 == unknown);

	printf("tell: %ld\n", ftell(anm));
}

int main(int argc, char* argv[])
{
	ce_logging_init();
#ifdef NDEBUG
	ce_logging_set_level(CE_LOGGING_LEVEL_WARNING);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif
	ce_alloc_init();

	if (argc == 2) {
		FILE* anm = fopen(argv[1], "rb");
		if (NULL == anm) {
			printf("main: failed to load anm: '%s'\n", argv[1]);
			return EXIT_FAILURE;
		}
		debug_print(anm);
		fclose(anm);
	}

	ce_alloc_term();
	ce_logging_term();

	return EXIT_SUCCESS;
}
