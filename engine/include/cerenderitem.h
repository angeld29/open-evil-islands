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

#ifndef CE_RENDERITEM_H
#define CE_RENDERITEM_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cevec3.h"
#include "cequat.h"
#include "ceaabb.h"
#include "cebbox.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_renderitem ce_renderitem;

typedef struct {
	void (*ctor)(ce_renderitem* renderitem, va_list args);   // required
	void (*dtor)(ce_renderitem* renderitem);                 // required
	void (*update)(ce_renderitem* renderitem, va_list args); // may be NULL
	void (*render)(ce_renderitem* renderitem);               // required
	void (*clone)(const ce_renderitem* renderitem,
					ce_renderitem* clone_renderitem);        // required
} ce_renderitem_vtable;

struct ce_renderitem {
	ce_aabb aabb;
	ce_vec3 position;
	ce_quat orientation;
	ce_bbox bbox;
	ce_vec3 world_position;
	ce_quat world_orientation;
	ce_bbox world_bbox;
	ce_renderitem_vtable vtable;
	size_t size;
	char impl[];
};

extern ce_renderitem* ce_renderitem_new(ce_renderitem_vtable vtable,
										size_t size, ...);
extern void ce_renderitem_del(ce_renderitem* renderitem);

extern void ce_renderitem_update(ce_renderitem* renderitem, ...);
extern void ce_renderitem_render(ce_renderitem* renderitem);

extern ce_renderitem* ce_renderitem_clone(const ce_renderitem* renderitem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERITEM_H */
