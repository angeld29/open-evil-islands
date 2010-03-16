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

#include <stdio.h>

#include "celogging.h"
#include "cealloc.h"
#include "ceroot.h"
#include "ceterrain.h"

static bool ce_terrain_create(ce_terrain* terrain,
								const char* zone_name,
								const ce_vec3* position,
								const ce_quat* orientation,
								ce_scenenode* parent_scenenode)
{
	if (NULL == (terrain->mprfile =
			ce_mprmng_open_mprfile(ce_root_get_mprmng(), zone_name))) {
		return false;
	}

	terrain->textures = ce_vector_new_reserved(terrain->mprfile->texture_count);

	if (NULL == (terrain->stub_texture =
			ce_texmng_get_texture(ce_root_get_texmng(), "default0"))) {
		return false;
	}
	ce_texture_add_ref(terrain->stub_texture);

	// mpr name + nnn
	char texture_name[terrain->mprfile->name->length + 3 + 1];

	for (int i = 0, n = terrain->mprfile->texture_count; i < n; ++i) {
		snprintf(texture_name, sizeof(texture_name), "%s%03d",
				ce_string_cstr(terrain->mprfile->name), i);

		ce_texture* texture =
			ce_texmng_get_texture(ce_root_get_texmng(), texture_name);
		if (NULL == texture) {
			return false;
		}

		ce_vector_push_back(terrain->textures, ce_texture_add_ref(texture));
	}

	if (NULL == (terrain->scenenode = ce_scenenode_new(parent_scenenode))) {
		return false;
	}

	ce_vec3_copy(&terrain->scenenode->position, position);
	ce_quat_copy(&terrain->scenenode->orientation, orientation);

	return ce_terrain_create_impl(terrain);
}

ce_terrain* ce_terrain_new(const char* zone_name,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_scenenode* parent_scenenode)
{
	ce_terrain* terrain = ce_alloc_zero(sizeof(ce_terrain));
	if (NULL == terrain) {
		ce_logging_error("terrain: could not allocate memory");
		return NULL;
	}

	if (!ce_terrain_create(terrain, zone_name, position,
							orientation, parent_scenenode)) {
		ce_terrain_del(terrain);
		return NULL;
	}

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_scenenode_del(terrain->scenenode);
		if (NULL != terrain->textures) {
			for (int i = 0; i < terrain->textures->count; ++i) {
				ce_texture_del(terrain->textures->items[i]);
			}
			ce_vector_del(terrain->textures);
		}
		ce_texture_del(terrain->stub_texture);
		ce_mprfile_close(terrain->mprfile);
		ce_free(terrain, sizeof(ce_terrain));
	}
}
