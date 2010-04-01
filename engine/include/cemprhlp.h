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

#ifndef CE_MPRHLP_H
#define CE_MPRHLP_H

#include <stdbool.h>

#include "ceaabb.h"
#include "cemprfile.h"
#include "cematerial.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern ce_aabb* ce_mprhlp_get_aabb(ce_aabb* aabb,
									const ce_mprfile* mprfile,
									int sector_x, int sector_z);

extern const ce_mprfile_material*
ce_mprhlp_find_material(const ce_mprfile* mprfile,
						ce_mprfile_material_type type);

extern float* ce_mprhlp_normal2vector(float* vector, uint32_t normal);

extern int ce_mprhlp_texture_index(uint16_t texture);
extern int ce_mprhlp_texture_number(uint16_t texture);
extern int ce_mprhlp_texture_angle(uint16_t texture);

extern float ce_mprhlp_get_height(const ce_mprfile* mprfile, float x, float z);

extern ce_material* ce_mprhlp_create_material(const ce_mprfile* mprfile,
											bool opacity, ce_texture* texture);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MPRHLP_H */
