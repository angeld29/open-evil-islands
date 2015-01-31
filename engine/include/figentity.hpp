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

#ifndef CE_FIGENTITY_HPP
#define CE_FIGENTITY_HPP

#include <stdbool.h>

#include "vec3.hpp"
#include "quat.hpp"
#include "vector.hpp"
#include "figmesh.hpp"
#include "figbone.hpp"
#include "texture.hpp"
#include "scenenode.hpp"
#include "renderqueue.hpp"

typedef struct {
    float height_correction;
    ce_vec3 position;
    ce_quat orientation;
    ce_figmesh* figmesh;
    ce_figbone* figbone;
    ce_vector* textures;
    ce_vector* renderlayers;
    ce_scenenode* scenenode;
} ce_figentity;

extern ce_figentity* ce_figentity_new(ce_figmesh* figmesh, const ce_vec3* position, const ce_quat* orientation, const char* parts[], const char* textures[], ce_scenenode* scenenode);
extern void ce_figentity_del(ce_figentity* figentity);

extern void ce_figentity_fix_height(ce_figentity* figentity, float height);

extern int ce_figentity_get_animation_count(ce_figentity* figentity);
extern const char* ce_figentity_get_animation_name(ce_figentity* figentity, int index);

extern bool ce_figentity_play_animation(ce_figentity* figentity, const char* name);
extern void ce_figentity_stop_animation(ce_figentity* figentity);

#endif /* CE_FIGENTITY_HPP */
