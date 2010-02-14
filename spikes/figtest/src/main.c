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

#include "celogging.h"
#include "cealloc.h"
#include "cefigfile.h"

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
		ce_figfile* fig = ce_figfile_open(argv[1]);
		if (NULL == fig) {
			printf("main: failed to load fig: '%s'\n", argv[1]);
		}
		printf("vertex_count: %d\n", fig->vertex_count);
		printf("normal_count: %d\n", fig->normal_count);
		printf("texcoord_count: %d\n", fig->texcoord_count);
		printf("index_count: %d\n", fig->index_count);
		printf("component_count: %d\n", fig->component_count);
		printf("light_component_count: %d\n", fig->light_component_count);
		printf("unknown1: %d\n", fig->unknown1);
		printf("unknown2: %d\n", fig->unknown2);
		printf("center:\n");
		for (int i = 0; i < 8; ++i) {
			printf("\t%f %f %f\n", fig->center[i].x, fig->center[i].y, fig->center[i].z);
		}
		printf("min:\n");
		for (int i = 0; i < 8; ++i) {
			printf("\t%f %f %f\n", fig->min[i].x, fig->min[i].y, fig->min[i].z);
		}
		printf("max:\n");
		for (int i = 0; i < 8; ++i) {
			printf("\t%f %f %f\n", fig->max[i].x, fig->max[i].y, fig->max[i].z);
		}
		printf("radius:\n");
		for (int i = 0; i < 8; ++i) {
			printf("\t%f\n", fig->radius[i]);
		}
		ce_figfile_close(fig);
	}

	ce_alloc_term();
	ce_logging_term();
	return EXIT_SUCCESS;
}
