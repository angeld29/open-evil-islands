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
		printf("unknown1: %d\n", fig->unknown1);
		printf("unknown2: %d\n", fig->unknown2);
		printf("\ncenter:\n");
		for (unsigned int i = 0; i < 8; ++i) {
			printf("\t%f %f %f\n", fig->center[i].x, fig->center[i].y, fig->center[i].z);
		}
		printf("\nmin:\n");
		for (unsigned int i = 0; i < 8; ++i) {
			printf("\t%f %f %f\n", fig->min[i].x, fig->min[i].y, fig->min[i].z);
		}
		printf("\nmax:\n");
		for (unsigned int i = 0; i < 8; ++i) {
			printf("\t%f %f %f\n", fig->max[i].x, fig->max[i].y, fig->max[i].z);
		}
		printf("\nradius:\n");
		for (unsigned int i = 0; i < 8; ++i) {
			printf("\t%f\n", fig->radius[i]);
		}
		printf("\nvertices (%u):\n\n", fig->vertex_count);
		for (unsigned int i = 0; i < fig->vertex_count; ++i) {
			printf("\tvertex %u:\n", i + 1);
			for (unsigned int j = 0; j < 8; ++j) {
				printf("\t\tx: %f %f %f %f\n", fig->vertices[i].x[j][0], fig->vertices[i].x[j][1], fig->vertices[i].x[j][2], fig->vertices[i].x[j][3]);
				printf("\t\ty: %f %f %f %f\n", fig->vertices[i].y[j][0], fig->vertices[i].y[j][1], fig->vertices[i].y[j][2], fig->vertices[i].y[j][3]);
				printf("\t\tz: %f %f %f %f\n\n", fig->vertices[i].z[j][0], fig->vertices[i].z[j][1], fig->vertices[i].z[j][2], fig->vertices[i].z[j][3]);
			}
		}
		printf("\nnormals (%u):\n", fig->normal_count);
		for (unsigned int i = 0; i < fig->normal_count; ++i) {
			printf("\tx: %f %f %f %f\n", fig->normals[i].x[0], fig->normals[i].x[1], fig->normals[i].x[2], fig->normals[i].x[3]);
			printf("\ty: %f %f %f %f\n", fig->normals[i].y[0], fig->normals[i].y[1], fig->normals[i].y[2], fig->normals[i].y[3]);
			printf("\tz: %f %f %f %f\n", fig->normals[i].z[0], fig->normals[i].z[1], fig->normals[i].z[2], fig->normals[i].z[3]);
			printf("\tw: %f %f %f %f\n\n", fig->normals[i].w[0], fig->normals[i].w[1], fig->normals[i].w[2], fig->normals[i].w[3]);
		}
		printf("texcoords (%u):\n", fig->texcoord_count);
		for (unsigned int i = 0; i < fig->texcoord_count; ++i) {
			printf("\t%f %f\n", fig->texcoords[i].x, fig->texcoords[i].y);
		}
		printf("\nindices (%u):\n\t", fig->index_count);
		for (unsigned int i = 0; i < fig->index_count; ++i) {
			printf("%hu ", fig->indices[i]);
		}
		printf("\n\ncomponents (%u):\n\t", fig->component_count);
		for (unsigned int i = 0; i < fig->component_count; ++i) {
			printf("(%hu, %hu, %hu) ", fig->components[i].a,
				fig->components[i].b, fig->components[i].c);
		}
		printf("\n\nlight components (%u):\n\t", fig->light_component_count);
		for (unsigned int i = 0; i < fig->light_component_count; ++i) {
			printf("(%hu, %hu) ", fig->light_components[i].a,
									fig->light_components[i].b);
		}
		printf("\n");
		ce_figfile_close(fig);
	}

	ce_alloc_term();
	ce_logging_term();
	return EXIT_SUCCESS;
}
