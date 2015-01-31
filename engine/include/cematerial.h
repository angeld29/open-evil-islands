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

#ifndef CE_MATERIAL_H
#define CE_MATERIAL_H

#include <stdbool.h>

#include "cecolor.h"
#include "ceshader.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
    CE_MATERIAL_MODE_MODULATE,
    CE_MATERIAL_MODE_DECAL,
    CE_MATERIAL_MODE_REPLACE,
    CE_MATERIAL_MODE_COUNT
} ce_material_mode;

typedef struct {
    ce_material_mode mode;
    ce_color ambient;
    ce_color diffuse;
    ce_color specular;
    ce_color emission;
    float shininess;
    bool alpha_test;
    bool blend;
    ce_shader* shader;
} ce_material;

extern ce_material* ce_material_new(void);
extern void ce_material_del(ce_material* material);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MATERIAL_H */
