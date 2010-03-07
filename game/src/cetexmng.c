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
#include <string.h>
#include <assert.h>

#include "celogging.h"
#include "cealloc.h"
#include "ceresfile.h"
#include "cetexmng.h"

ce_texmng* ce_texmng_new(void)
{
	ce_texmng* texmng = ce_alloc_zero(sizeof(ce_texmng));
	if (NULL == texmng) {
		ce_logging_error("texmng: could not allocate memory");
		return NULL;
	}

	if (NULL == (texmng->resources = ce_vector_new()) ||
			NULL == (texmng->textures = ce_vector_new())) {
		ce_texmng_del(texmng);
		return NULL;
	}

	return texmng;
}

void ce_texmng_del(ce_texmng* texmng)
{
	if (NULL != texmng) {
		assert(0 == ce_vector_count(texmng->textures));
		for (int i = 0, n = ce_vector_count(texmng->resources); i < n; ++i) {
			ce_resfile_close(ce_vector_at(texmng->resources, i));
		}
		ce_vector_del(texmng->textures);
		ce_vector_del(texmng->resources);
		ce_free(texmng, sizeof(ce_texmng));
	}
}

bool ce_texmng_register_resource(ce_texmng* texmng, const char* path)
{
	ce_resfile* resfile = ce_resfile_open_file(path);
	if (NULL == resfile) {
		ce_logging_error("texmng: could not open resource: '%s'", path);
		return false;
	}

	ce_vector_push_back(texmng->resources, resfile);
	ce_logging_write("texmng: loading '%s'... ok", ce_resfile_name(resfile));

	return true;
}

ce_texture* ce_texmng_acquire_texture(ce_texmng* texmng, const char* name)
{
	char full_name[strlen(name) + 4 + 1];
	snprintf(full_name, sizeof(full_name), "%s.mmp", name);

	for (int i = 0, n = ce_vector_count(texmng->textures); i < n; ++i) {
		ce_texture* texture = ce_vector_at(texmng->textures, i);
		if (0 == strcmp(full_name, ce_texture_get_name(texture))) {
			ce_texture_inc_ref(texture);
			return texture;
		}
	}

	for (int i = 0, n = ce_vector_count(texmng->resources); i < n; ++i) {
		ce_resfile* resfile = ce_vector_at(texmng->resources, i);
		int index = ce_resfile_node_index(resfile, full_name);
		if (-1 != index) {
			const size_t data_size = ce_resfile_node_size(resfile, index);
			void* data = ce_alloc(data_size);
			if (NULL == data) {
				ce_logging_error("texmng: could not allocate memory");
				return false;
			}

			if (!ce_resfile_node_data(resfile, index, data)) {
				ce_free(data, data_size);
				return false;
			}

			ce_texture* texture = ce_texture_new(full_name, data);
			ce_free(data, data_size);

			if (NULL != texture) {
				ce_vector_push_back(texmng->textures, texture);
				ce_texture_inc_ref(texture);
			}

			return texture;
		}
	}

	ce_logging_error("texmng: could not find texture: '%s'", full_name);
	return NULL;
}

void ce_texmng_release_texture(ce_texmng* texmng, ce_texture* texture)
{
	for (int i = 0, n = ce_vector_count(texmng->textures); i < n; ++i) {
		if (ce_vector_at(texmng->textures, i) == texture) {
			ce_texture_dec_ref(texture);
			if (1 == ce_texture_get_ref_count(texture)) {
				ce_texture_del(texture);
				ce_vector_replace(texmng->textures, i,
								ce_vector_back(texmng->textures));
				ce_vector_pop_back(texmng->textures);
			}
			return;
		}
	}
	assert(false);
}
