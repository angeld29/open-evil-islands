/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cassert>

#include "logging.hpp"
#include "fighelpers.hpp"

namespace cursedearth
{
    aabb_t* ce_fighlp_get_aabb(aabb_t* aabb, const ce_figfile* figfile, const complection_t* complection)
    {
        if (0 == figfile->vertex_count) {
            // I found some strange figure files!
            // all bounds in these ones are NAN or INF...
            return ce_aabb_init_zero(aabb);
        }

        aabb->radius = figfile->value_callback(figfile->radius, 1, complection);

        ce_vec3_init(&aabb->origin,
            figfile->value_callback(figfile->center + 0, 3, complection),
            figfile->value_callback(figfile->center + 1, 3, complection),
            figfile->value_callback(figfile->center + 2, 3, complection));

        vector3_t min, max;
        ce_vec3_init(&min,
            figfile->value_callback(figfile->min + 0, 3, complection),
            figfile->value_callback(figfile->min + 1, 3, complection),
            figfile->value_callback(figfile->min + 2, 3, complection));
        ce_vec3_init(&max,
            figfile->value_callback(figfile->max + 0, 3, complection),
            figfile->value_callback(figfile->max + 1, 3, complection),
            figfile->value_callback(figfile->max + 2, 3, complection));

        // add extra space to guarantee full coating of the object
        const float extra = 0.1f;

        ce_vec3_sub(&aabb->extents, &max, &min);
        ce_vec3_scale(&aabb->extents, 0.5f + extra, &aabb->extents);

        return aabb;
    }

    sphere_t* ce_fighlp_get_sphere(sphere_t* sphere, const ce_figfile* figfile, const complection_t* complection)
    {
        vector3_t center;
        return ce_sphere_init(sphere, ce_vec3_init(&center,
            figfile->value_callback(figfile->center + 0, 3, complection),
            figfile->value_callback(figfile->center + 1, 3, complection),
            figfile->value_callback(figfile->center + 2, 3, complection)),
            figfile->value_callback(figfile->radius, 1, complection));
    }

    float* ce_fighlp_get_vertex(float* array, const ce_figfile* figfile, int index, const complection_t* complection)
    {
        // see doc/formats/figfile.txt for details!
        size_t i = 3 * figfile->value_count * 4 * (index / 4) + index % 4;
        size_t j = 3 * figfile->value_count * 4 / 3;
        array[0] = figfile->value_callback(figfile->vertices + i + 0 * j, 4, complection);
        array[1] = figfile->value_callback(figfile->vertices + i + 1 * j, 4, complection);
        array[2] = figfile->value_callback(figfile->vertices + i + 2 * j, 4, complection);
        return array;
    }

    float* ce_fighlp_get_normal(float* array, const ce_figfile* figfile, int index)
    {
        size_t i = 4 * 4 * (index / 4) + index % 4;
        float inv_w = 1.0f / figfile->normals[i + 12];
        array[0] = figfile->normals[i + 0] * inv_w;
        array[1] = figfile->normals[i + 4] * inv_w;
        array[2] = figfile->normals[i + 8] * inv_w;
        return array;
    }

    vector3_t* ce_fighlp_get_bone(vector3_t* position, const ce_figfile* figfile, const ce_bonfile* bonfile, const complection_t* complection)
    {
        return ce_vec3_init(position,
            figfile->value_callback(bonfile->bone + 0, 3, complection),
            figfile->value_callback(bonfile->bone + 1, 3, complection),
            figfile->value_callback(bonfile->bone + 2, 3, complection));
    }

    ce_material* ce_fighlp_create_material(const ce_figfile* figfile)
    {
        ce_material* material = ce_material_new();
        material->mode = CE_MATERIAL_MODE_REPLACE;

        switch (figfile->material_group) {
        case 17: // unmoli, unmosp
            material->alpha_test = true;
            break;
        case 18: // nafltr59, stst82, unmosk
            material->alpha_test = true;
            break;
        case 19: // unhuma, unhufe, unmozo0, unmozo1
            break;
        case 21: // nafltr82, unmosu, unmocy, unmodg, unmogo
            material->alpha_test = true;
            break;
        case 22: // nast10, stbuho62, stwa1
            break;
        case 23: // unmoel1, unmosh
            material->alpha_test = true;
            break;
        case 25: // unmowi
            material->blend = true;
            break;
        default:
            assert(false);
        }

        return material;
    }
}
