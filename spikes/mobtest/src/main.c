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
#include "cemobfile.h"

static void debug_print(ce_mobfile* mob)
{
	if (NULL != mob->script) {
		printf("script (%d bytes):\n%s\n", (int)ce_string_length(mob->script),
											ce_string_cstr(mob->script));
	}
	printf("objects (%d):\n", (int)ce_vector_count(mob->objects));
	for (int i = 0, n = ce_vector_count(mob->objects); i < n; ++i) {
		ce_mobobject_object* obj = ce_vector_at(mob->objects, i);
		printf("\tparts (%d):\n", (int)ce_vector_count(obj->parts));
		if (0 != ce_vector_count(obj->parts)) {
			printf("\t\t");
		}
		for (int j = 0, m = ce_vector_count(obj->parts); j < m; ++j) {
			ce_string* str = ce_vector_at(obj->parts, j);
			printf("%s ", ce_string_cstr(str));
		}
		if (0 != ce_vector_count(obj->parts)) {
			printf("\n");
		}
		printf("\towner: %d\n", obj->owner);
		printf("\tid: %d\n", obj->id);
		printf("\ttype: %d\n", obj->type);
		printf("\tname: %s\n", ce_string_cstr(obj->name));
		printf("\tmodel_name: %s\n", ce_string_cstr(obj->model_name));
		printf("\tparent_name: %s\n", ce_string_cstr(obj->parent_name));
		printf("\tprimary_texture: %s\n", ce_string_cstr(obj->primary_texture));
		printf("\tsecondary_texture: %s\n", ce_string_cstr(obj->secondary_texture));
		printf("\tcomment: %s\n", ce_string_cstr(obj->comment));
		printf("\tposition: %f %f %f\n", obj->position.x, obj->position.y, obj->position.z);
		printf("\trotation: %f %f %f %f\n", obj->rotation.w, obj->rotation.x, obj->rotation.y, obj->rotation.z);
		printf("\tquest: %s\n", obj->quest ? "yes" : "no");
		printf("\tshadow: %s\n", obj->shadow ? "yes" : "no");
		printf("\tparent_id: %d\n", obj->parent_id);
		printf("\tquest_info: %s\n", ce_string_cstr(obj->quest_info));
		printf("\tcomplection: %f %f %f\n\n", obj->complection.strength, obj->complection.dexterity, obj->complection.tallness);
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
		ce_mobfile* mob = ce_mobfile_open(argv[1]);
		if (NULL == mob) {
			printf("main: failed to load mob: '%s'\n", argv[1]);
			return EXIT_FAILURE;
		}
		debug_print(mob);
		ce_mobfile_close(mob);
	}

	ce_alloc_term();
	ce_logging_term();

	return EXIT_SUCCESS;
}
