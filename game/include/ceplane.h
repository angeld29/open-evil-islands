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

#ifndef CE_PLANE_H
#define CE_PLANE_H

#include "cevec3.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vec3 n;
	float d;
} ce_plane;

extern ce_plane* ce_plane_init(ce_plane* r, float a, float b, float c, float d);
extern ce_plane* ce_plane_init_array(ce_plane* r, float* v);
extern ce_plane* ce_plane_init_tri(ce_plane* r, const ce_vec3* a,
									const ce_vec3* b, const ce_vec3* c);

extern ce_plane* ce_plane_normalise(ce_plane* r, const ce_plane* a);

extern float ce_plane_dist(const ce_plane* a, const ce_vec3* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_PLANE_H */
