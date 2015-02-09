/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>
#include <cstring>
#include <vector>

#include "alloc.hpp"
#include "str.hpp"
#include "path.hpp"
#include "logging.hpp"
#include "resfile.hpp"
#include "optionmanager.hpp"
#include "fighelpers.hpp"
#include "figuremanager.hpp"

namespace cursedearth
{
    struct ce_figure_manager* ce_figure_manager;

    const char* ce_figure_exts[] = { ".mod", NULL };
    const char* ce_figure_resource_dirs[] = { "Res", NULL };
    const char* ce_figure_resource_exts[] = { ".res", NULL };
    const char* ce_figure_resource_names[] = { "figures", "menus", NULL };

    void ce_notify_figproto_created(ce_vector* listeners, ce_figproto* figproto)
    {
        for (size_t i = 0; i < listeners->count; ++i) {
            ce_figure_manager_listener* listener = (ce_figure_manager_listener*)listeners->items[i];
            if (NULL != listener->figproto_created) {
                (*listener->figproto_created)(listener->listener, figproto);
            }
        }
    }

    void ce_notify_figmesh_created(ce_vector* listeners, ce_figmesh* figmesh)
    {
        for (size_t i = 0; i < listeners->count; ++i) {
            ce_figure_manager_listener* listener = (ce_figure_manager_listener*)listeners->items[i];
            if (NULL != listener->figmesh_created) {
                (*listener->figmesh_created)(listener->listener, figmesh);
            }
        }
    }

    // TODO: cleanup unused protos and meshes

    void ce_figure_manager_init(void)
    {
        ce_figure_manager = (struct ce_figure_manager*)ce_alloc_zero(sizeof(struct ce_figure_manager));
        ce_figure_manager->res_files = ce_vector_new();
        ce_figure_manager->figprotos = ce_vector_new();
        ce_figure_manager->figmeshes = ce_vector_new();
        ce_figure_manager->entities = ce_vector_new_reserved(512);
        ce_figure_manager->listeners = ce_vector_new();

        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 32);

        for (size_t i = 0; NULL != ce_figure_resource_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_figure_resource_dirs[i], NULL);
            ce_logging_info("figure manager: using path `%s'", path.data());
        }

        for (size_t i = 0; NULL != ce_figure_resource_names[i]; ++i) {
            ce_res_file* res_file;
            if (NULL != ce_path_find_special1(path.data(), path.size(), option_manager_t::instance()->ei_path2->str,
                    ce_figure_resource_names[i], ce_figure_resource_dirs, ce_figure_resource_exts) &&
                    NULL != (res_file = ce_res_file_new_path(path.data()))) {
                ce_vector_push_back(ce_figure_manager->res_files, res_file);
                ce_logging_info("figure manager: loading `%s'... ok", path.data());
            } else {
                ce_logging_error("figure manager: loading `%s'... failed", path.data());
            }
        }
    }

    void ce_figure_manager_term(void)
    {
        if (NULL != ce_figure_manager) {
            ce_figure_manager_clear();
            ce_vector_for_each(ce_figure_manager->figmeshes, (void(*)(void*))ce_figmesh_del);
            ce_vector_for_each(ce_figure_manager->figprotos, (void(*)(void*))ce_figproto_del);
            ce_vector_for_each(ce_figure_manager->res_files, (void(*)(void*))ce_res_file_del);
            ce_vector_del(ce_figure_manager->listeners);
            ce_vector_del(ce_figure_manager->entities);
            ce_vector_del(ce_figure_manager->figmeshes);
            ce_vector_del(ce_figure_manager->figprotos);
            ce_vector_del(ce_figure_manager->res_files);
            ce_free(ce_figure_manager, sizeof(struct ce_figure_manager));
        }
    }

    void ce_figure_manager_clear(void)
    {
        ce_vector_for_each(ce_figure_manager->entities, (void(*)(void*))ce_figentity_del);
        ce_vector_clear(ce_figure_manager->entities);
    }

    ce_figproto* ce_figure_manager_create_proto(const char* name)
    {
        std::vector<char> true_name(strlen(name) + 1);
        ce_path_remove_ext(true_name.data(), name);

        // find in cache
        for (size_t i = 0; i < ce_figure_manager->figprotos->count; ++i) {
            ce_figproto* figproto = (ce_figproto*)ce_figure_manager->figprotos->items[i];
            if (0 == ce_strcasecmp(true_name.data(), figproto->name->str)) {
                return figproto;
            }
        }

        std::vector<char> file_name(strlen(name) + 8);
        ce_path_append_ext(file_name.data(), file_name.size(), name, ce_figure_exts[0]);

        for (size_t i = 0; i < ce_figure_manager->res_files->count; ++i) {
            ce_res_file* res_file = (ce_res_file*)ce_figure_manager->res_files->items[i];
            if (res_file->node_count != ce_res_file_node_index(res_file, file_name.data())) {
                ce_figproto* figproto = ce_figproto_new(true_name.data(), res_file);
                ce_vector_push_back(ce_figure_manager->figprotos, figproto);
                ce_notify_figproto_created(ce_figure_manager->listeners, figproto);
                return figproto;
            }
        }

        ce_logging_error("figure manager: could not create figure proto `%s'", name);
        return NULL;
    }

    ce_figmesh* ce_figure_manager_create_mesh(const char* name, const complection_t* complection)
    {
        std::vector<char> true_name(strlen(name) + 1);
        ce_path_remove_ext(true_name.data(), name);

        for (size_t i = 0; i < ce_figure_manager->figmeshes->count; ++i) {
            ce_figmesh* figmesh = (ce_figmesh*)ce_figure_manager->figmeshes->items[i];
            if (0 == ce_strcasecmp(true_name.data(), figmesh->figproto->name->str) && ce_complection_equal(complection, &figmesh->complection)) {
                return figmesh;
            }
        }

        ce_figproto* figproto = ce_figure_manager_create_proto(name);
        if (NULL != figproto) {
            ce_figmesh* figmesh = ce_figmesh_new(figproto, complection);
            ce_vector_push_back(ce_figure_manager->figmeshes, figmesh);
            ce_notify_figmesh_created(ce_figure_manager->listeners, figmesh);
            return figmesh;
        }

        ce_logging_error("figure manager: could not create figure mesh `%s'", name);
        return NULL;
    }

    ce_figentity* ce_figure_manager_create_entity(const char* name, const complection_t* complection, const vector3_t* position, const quaternion_t* orientation, const char* parts[], const char* textures[])
    {
        ce_figmesh* mesh = ce_figure_manager_create_mesh(name, complection);
        if (NULL != mesh) {
            ce_figentity* entity = ce_figentity_new(mesh, position, orientation, parts, textures, NULL);
            if (NULL != entity) {
                ce_vector_push_back(ce_figure_manager->entities, entity);
                return entity;
            }
        }

        ce_logging_error("figure manager: could not create figure entity `%s'", name);
        return NULL;
    }

    void ce_figure_manager_remove_entity(ce_figentity* entity)
    {
        ce_vector_remove_all(ce_figure_manager->entities, entity);
        ce_figentity_del(entity);
    }
}
