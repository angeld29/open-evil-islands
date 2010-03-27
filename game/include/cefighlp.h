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

#ifndef CE_FIGHLP_H
#define CE_FIGHLP_H

#include "cevec3.h"
#include "ceaabb.h"
#include "cesphere.h"
#include "cecomplection.h"
#include "cefigfile.h"
#include "cebonfile.h"
#include "cematerial.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern ce_aabb*
ce_fighlp_get_aabb(ce_aabb* aabb, const ce_figfile* figfile,
					const ce_complection* complection);

extern ce_sphere*
ce_fighlp_get_sphere(ce_sphere* sphere, const ce_figfile* figfile,
					const ce_complection* complection);

extern float*
ce_fighlp_get_vertex(float* array, const ce_figfile* figfile,
						int index, const ce_complection* complection);

extern float*
ce_fighlp_get_normal(float* array, const ce_figfile* figfile, int index);

extern ce_vec3*
ce_fighlp_get_bone(ce_vec3* position,
					const ce_figfile* figfile,
					const ce_bonfile* bonfile,
					const ce_complection* complection);

extern ce_material* ce_fighlp_create_material(ce_texture* texture);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGHLP_H */
