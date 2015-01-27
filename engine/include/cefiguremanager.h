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

#ifndef CE_FIGUREMANAGER_H
#define CE_FIGUREMANAGER_H

#include <stdbool.h>

#include "cevector.h"
#include "cefigproto.h"
#include "cefigmesh.h"
#include "cefigentity.h"

#ifdef __cplusplus
extern "C" {
#endif

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

extern void ce_figure_manager_init(void);
extern void ce_figure_manager_term(void);

extern void ce_figure_manager_clear(void);

static inline void ce_figure_manager_add_listener(ce_figure_manager_listener* listener)
{
    ce_vector_push_back(ce_figure_manager->listeners, listener);
}

extern ce_figproto* ce_figure_manager_create_proto(const char* name);

extern ce_figmesh* ce_figure_manager_create_mesh(const char* name,
                                                const ce_complection* complection);

extern ce_figentity* ce_figure_manager_create_entity(const char* name,
    const ce_complection* complection,
    const ce_vec3* position, const ce_quat* orientation,
    const char* parts[], const char* textures[]);

extern void ce_figure_manager_remove_entity(ce_figentity* entity);

#ifdef __cplusplus
}
#endif

#endif /* CE_FIGUREMANAGER_H */
