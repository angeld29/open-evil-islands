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

#ifndef CE_FIGHELPERS_HPP
#define CE_FIGHELPERS_HPP

#include "vector3.hpp"
#include "aabb.hpp"
#include "sphere.hpp"
#include "complection.hpp"
#include "figfile.hpp"
#include "bonfile.hpp"
#include "material.hpp"

namespace cursedearth
{
    aabb_t* ce_fighlp_get_aabb(aabb_t* aabb, const ce_figfile* figfile, const complection_t* complection);
    sphere_t* ce_fighlp_get_sphere(sphere_t* sphere, const ce_figfile* figfile, const complection_t* complection);

    float* ce_fighlp_get_vertex(float* array, const ce_figfile* figfile, int index, const complection_t* complection);
    float* ce_fighlp_get_normal(float* array, const ce_figfile* figfile, int index);

    vector3_t* ce_fighlp_get_bone(vector3_t* position, const ce_figfile* figfile, const ce_bonfile* bonfile, const complection_t* complection);
    ce_material* ce_fighlp_create_material(const ce_figfile* figfile);
}

#endif
