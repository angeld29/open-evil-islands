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

#include "cestr.h"
#include "celogging.h"
#include "cealloc.h"
#include "ceresfile.h"
#include "cetexmng.h"

ce_texmng* ce_texmng_new(const char* path)
{
	ce_texmng* texmng = ce_alloc_zero(sizeof(ce_texmng));
	texmng->path = ce_string_new_str(path);
	texmng->resfiles = ce_vector_new();
	texmng->textures = ce_vector_new();
	ce_logging_write("texmng: root path: '%s'", path);
	return texmng;
}

void ce_texmng_del(ce_texmng* texmng)
{
	if (NULL != texmng) {
		ce_vector_for_each(texmng->textures, (ce_vector_func1)ce_texture_del);
		ce_vector_for_each(texmng->resfiles, (ce_vector_func1)ce_resfile_close);
		ce_vector_del(texmng->textures);
		ce_vector_del(texmng->resfiles);
		ce_string_del(texmng->path);
		ce_free(texmng, sizeof(ce_texmng));
	}
}

bool ce_texmng_register_resource(ce_texmng* texmng, const char* path)
{
	ce_resfile* resfile = ce_resfile_open_file(path);
	if (NULL == resfile) {
		ce_logging_error("texmng: loading '%s'... failed", path);
		return false;
	}

	ce_vector_push_back(texmng->resfiles, resfile);
	ce_logging_write("texmng: loading '%s'... ok", path);
	return true;
}

ce_mmpfile* ce_texmng_open_mmpfile(ce_texmng* texmng, const char* name)
{
	char file_name[strlen(name) + 4 + 1];
	snprintf(file_name, sizeof(file_name), "%s.mmp", name);

	char path[texmng->path->length + strlen(file_name) + 1 + 1];
	snprintf(path, sizeof(path), "%s/%s", texmng->path->str, file_name);

	// first, try to load from cache dir
	ce_mmpfile* mmpfile = ce_mmpfile_new_file(path);
	if (NULL != mmpfile) {
		// all mmp's in cache dir are ready to use
		return mmpfile;
	}

	// find in resources
	for (int i = 0; i < texmng->resfiles->count; ++i) {
		ce_resfile* resfile = texmng->resfiles->items[i];
		int index = ce_resfile_node_index(resfile, file_name);
		if (-1 != index) {
			ce_mmpfile* mmpfile = ce_mmpfile_new_resfile(resfile, index);
			if (CE_MMPFILE_FORMAT_PNT3 == mmpfile->format) {
				ce_mmpfile* mmpfile2 =
					ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_ARGB8);
				ce_mmpfile_del(mmpfile);
				ce_texmng_save_mmpfile(texmng, name, mmpfile = mmpfile2);
			}
			return mmpfile;
		}
	}

	return NULL;
}

void ce_texmng_save_mmpfile(ce_texmng* texmng,
							const char* name,
							ce_mmpfile* mmpfile)
{
	char path[texmng->path->length + strlen(name) + 5 + 1];
	snprintf(path, sizeof(path), "%s/%s.mmp", texmng->path->str, name);
	ce_mmpfile_save(mmpfile, path);
}

ce_texture* ce_texmng_acquire_texture(ce_texmng* texmng, const char* name)
{
	// first, find texture in cache
	for (int i = 0; i < texmng->textures->count; ++i) {
		ce_texture* texture = texmng->textures->items[i];
		if (0 == ce_strcasecmp(name, ce_texture_get_name(texture))) {
			return texture;
		}
	}

	// load texture from resources
	ce_mmpfile* mmpfile = ce_texmng_open_mmpfile(texmng, name);
	if (NULL != mmpfile) {
		ce_texture* texture = ce_texture_new(name, mmpfile);
		ce_mmpfile_del(mmpfile);
		ce_vector_push_back(texmng->textures, texture);
		return texture;
	}

	ce_logging_error("texmng: could not find texture: '%s'", name);
	return NULL;
}

ce_texture* ce_texmng_acquire_texture_mmpfile(ce_texmng* texmng,
											const char* name,
											ce_mmpfile* mmpfile)
{
	ce_texture* texture = ce_texture_new(name, mmpfile);
	ce_vector_push_back(texmng->textures, texture);
	return texture;
}
