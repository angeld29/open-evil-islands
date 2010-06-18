/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include "celib.h"
#include "cestr.h"
#include "cepath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceresfile.h"
#include "ceoptionmanager.h"
#include "cefighlp.h"
#include "cefiguremanager.h"

struct ce_figure_manager* ce_figure_manager;

static const char* ce_figure_exts[] = {".mod", NULL};
static const char* ce_figure_resource_dirs[] = {"Res", NULL};
static const char* ce_figure_resource_exts[] = {".res", NULL};
static const char* ce_figure_resource_names[] = {"figures", "menus", NULL};

static void ce_notify_figproto_created(ce_vector* listeners, ce_figproto* figproto)
{
	for (size_t i = 0; i < listeners->count; ++i) {
		ce_figure_manager_listener* listener = listeners->items[i];
		if (NULL != listener->figproto_created) {
			(*listener->figproto_created)(listener->listener, figproto);
		}
	}
}

static void ce_notify_figmesh_created(ce_vector* listeners, ce_figmesh* figmesh)
{
	for (size_t i = 0; i < listeners->count; ++i) {
		ce_figure_manager_listener* listener = listeners->items[i];
		if (NULL != listener->figmesh_created) {
			(*listener->figmesh_created)(listener->listener, figmesh);
		}
	}
}

// TODO: cleanup unused protos and meshes

void ce_figure_manager_init(void)
{
	ce_figure_manager = ce_alloc_zero(sizeof(struct ce_figure_manager));
	ce_figure_manager->resfiles = ce_vector_new();
	ce_figure_manager->figprotos = ce_vector_new();
	ce_figure_manager->figmeshes = ce_vector_new();
	ce_figure_manager->listeners = ce_vector_new();

	char path[ce_option_manager->ei_path->length + 32];

	for (size_t i = 0; NULL != ce_figure_resource_dirs[i]; ++i) {
		ce_path_join(path, sizeof(path),
			ce_option_manager->ei_path->str, ce_figure_resource_dirs[i], NULL);
		ce_logging_write("figure manager: using path '%s'", path);
	}

	for (size_t i = 0; NULL != ce_figure_resource_names[i]; ++i) {
		ce_resfile* resfile;
		if (NULL != ce_path_find_special1(path, sizeof(path),
				ce_option_manager->ei_path->str, ce_figure_resource_names[i],
				ce_figure_resource_dirs, ce_figure_resource_exts) &&
				NULL != (resfile = ce_resfile_open_file(path))) {
			ce_vector_push_back(ce_figure_manager->resfiles, resfile);
			ce_logging_write("figure manager: loading '%s'... ok", path);
		} else {
			ce_logging_error("figure manager: loading '%s'... failed", path);
		}
	}
}

void ce_figure_manager_term(void)
{
	if (NULL != ce_figure_manager) {
		ce_vector_for_each(ce_figure_manager->figmeshes, ce_figmesh_del);
		ce_vector_for_each(ce_figure_manager->figprotos, ce_figproto_del);
		ce_vector_for_each(ce_figure_manager->resfiles, ce_resfile_close);
		ce_vector_del(ce_figure_manager->listeners);
		ce_vector_del(ce_figure_manager->figmeshes);
		ce_vector_del(ce_figure_manager->figprotos);
		ce_vector_del(ce_figure_manager->resfiles);
		ce_free(ce_figure_manager, sizeof(struct ce_figure_manager));
	}
}

static ce_figproto* ce_figure_manager_get_figproto(const char* name)
{
	char true_name[strlen(name) + 1];
	ce_path_remove_ext(true_name, name);

	// find in cache
	for (size_t i = 0; i < ce_figure_manager->figprotos->count; ++i) {
		ce_figproto* figproto = ce_figure_manager->figprotos->items[i];
		if (0 == ce_strcasecmp(true_name, figproto->name->str)) {
			return figproto;
		}
	}

	char file_name[strlen(name) + 8];
	ce_path_append_ext(file_name, sizeof(file_name),
						name, ce_figure_exts[0]);

	for (size_t i = 0; i < ce_figure_manager->resfiles->count; ++i) {
		ce_resfile* resfile = ce_figure_manager->resfiles->items[i];
		if (-1 != ce_resfile_node_index(resfile, file_name)) {
			ce_figproto* figproto = ce_figproto_new(true_name, resfile);
			ce_vector_push_back(ce_figure_manager->figprotos, figproto);
			ce_notify_figproto_created(ce_figure_manager->listeners, figproto);
			return figproto;
		}
	}

	ce_logging_error("figure manager: could not create figure proto '%s'", name);
	return NULL;
}

static ce_figmesh* ce_figure_manager_get_figmesh(const char* name,
								const ce_complection* complection)
{
	char true_name[strlen(name) + 1];
	ce_path_remove_ext(true_name, name);

	for (size_t i = 0; i < ce_figure_manager->figmeshes->count; ++i) {
		ce_figmesh* figmesh = ce_figure_manager->figmeshes->items[i];
		if (0 == ce_strcasecmp(true_name, figmesh->figproto->name->str) &&
				ce_complection_equal(complection, &figmesh->complection)) {
			return figmesh;
		}
	}

	ce_figproto* figproto = ce_figure_manager_get_figproto(name);
	if (NULL != figproto) {
		ce_figmesh* figmesh = ce_figmesh_new(figproto, complection);
		ce_vector_push_back(ce_figure_manager->figmeshes, figmesh);
		ce_notify_figmesh_created(ce_figure_manager->listeners, figmesh);
		return figmesh;
	}

	ce_logging_error("figure manager: could not create figure mesh '%s'", name);
	return NULL;
}

ce_figentity* ce_figure_manager_create_figentity(const char* name,
	const ce_complection* complection, const ce_vec3* position,
	const ce_quat* orientation, ce_vector* parts,
	int texture_count, ce_texture* textures[], ce_scenenode* scenenode)
{
	ce_figmesh* figmesh = ce_figure_manager_get_figmesh(name, complection);
	if (NULL == figmesh) {
		return NULL;
	}
	return ce_figentity_new(figmesh, position, orientation, parts,
							texture_count, textures, scenenode);
}
