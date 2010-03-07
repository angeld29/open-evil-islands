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

#ifndef CE_RENDLAYER_H
#define CE_RENDLAYER_H

#include "ceaabb.h"
#include "cesphere.h"
#include "cevector.h"
#include "cerenditem.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_aabb bounding_box;
	ce_sphere bounding_sphere;
	ce_vector* renditems;
} ce_rendlayer;

extern ce_rendlayer* ce_rendlayer_new(int renditem_count);
extern void ce_rendlayer_del(ce_rendlayer* rendlayer);

extern void ce_rendlayer_add_renditem(ce_rendlayer* rendlayer,
									ce_renditem* renditem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDLAYER_H */
