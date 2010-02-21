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

static void debug_print_item(FILE* lnk)
{
	uint32_t length;
	fread(&length, sizeof(uint32_t), 1, lnk);
	printf("\tlength: %u\n", length);
	char data[length];
	fread(data, 1, length, lnk);
	printf("\t");
	for (int j = 0, m = length; j < m && '\0' != data[j]; ++j) {
		printf("%c", data[j]);
	}
	printf("\n");
}

static void debug_print(FILE* lnk)
{
	uint32_t count;
	fread(&count, sizeof(uint32_t), 1, lnk);
	printf("count: %u\n", count);
	for (int i = 0, n = count; i < n; ++i) {
		printf("item1:\n");
		debug_print_item(lnk);
		printf("item2:\n");
		debug_print_item(lnk);
	}
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
		FILE* lnk = fopen(argv[1], "rb");
		if (NULL == lnk) {
			printf("main: failed to load lnk: '%s'\n", argv[1]);
			return EXIT_FAILURE;
		}
		debug_print(lnk);
		fclose(lnk);
	}

	ce_alloc_term();
	ce_logging_term();

	return EXIT_SUCCESS;
}
