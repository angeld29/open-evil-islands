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

#ifndef CE_FIGUREMANAGER_HPP
#define CE_FIGUREMANAGER_HPP

#include "vector.hpp"
#include "figproto.hpp"
#include "figmesh.hpp"
#include "figentity.hpp"

namespace cursedearth
{
    typedef struct {
        void (*figproto_created)(void* listener, ce_figproto* figproto);
        void (*figmesh_created)(void* listener, ce_figmesh* figmesh);
        void* listener;
    } ce_figure_manager_listener;

    extern struct ce_figure_manager {
        ce_vector* res_files;
        ce_vector* figprotos;
        ce_vector* figmeshes;
        ce_vector* entities;
        ce_vector* listeners;
    }* ce_figure_manager;

    void ce_figure_manager_init();
    void ce_figure_manager_term();
    void ce_figure_manager_clear();

    inline void ce_figure_manager_add_listener(ce_figure_manager_listener* listener)
    {
        ce_vector_push_back(ce_figure_manager->listeners, listener);
    }

    ce_figproto* ce_figure_manager_create_proto(const std::string& name);
    ce_figmesh* ce_figure_manager_create_mesh(const std::string& name, const complection_t* complection);
    ce_figentity* ce_figure_manager_create_entity(const std::string& name, const complection_t* complection, const vector3_t* position, const quaternion_t* orientation, const char* parts[], const char* textures[]);

    void ce_figure_manager_remove_entity(ce_figentity* entity);
}

#endif
