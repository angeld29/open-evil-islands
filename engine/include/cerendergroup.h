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

#ifndef CE_RENDERGROUP_H
#define CE_RENDERGROUP_H

#include "cevector.h"
#include "cematerial.h"
#include "cetexture.h"
#include "cerenderlayer.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	int priority;
	ce_material* material;
	ce_vector* renderlayers;
} ce_rendergroup;

extern ce_rendergroup* ce_rendergroup_new(int priority, ce_material* material);
extern void ce_rendergroup_del(ce_rendergroup* rendergroup);

extern void ce_rendergroup_clear(ce_rendergroup* rendergroup);

extern ce_renderlayer* ce_rendergroup_get(ce_rendergroup* rendergroup,
												ce_texture* texture);

extern void ce_rendergroup_render(ce_rendergroup* rendergroup);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERGROUP_H */
