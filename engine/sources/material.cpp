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

#include "alloc.hpp"
#include "material.hpp"

namespace cursedearth
{
    ce_material* ce_material_new(void)
    {
        ce_material* material = (ce_material*)ce_alloc_zero(sizeof(ce_material));
        material->mode = CE_MATERIAL_MODE_MODULATE;
        ce_color_init(&material->ambient, 0.2f, 0.2f, 0.2f, 1.0f);
        ce_color_init(&material->diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
        ce_color_init(&material->specular, 0.0f, 0.0f, 0.0f, 1.0f);
        ce_color_init(&material->emission, 0.0f, 0.0f, 0.0f, 1.0f);
        return material;
    }

    void ce_material_del(ce_material* material)
    {
        if (NULL != material) {
            ce_shader_del(material->shader);
            ce_free(material, sizeof(ce_material));
        }
    }
}
