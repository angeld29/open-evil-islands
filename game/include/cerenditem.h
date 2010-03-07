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

#ifndef CE_RENDITEM_H
#define CE_RENDITEM_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "ceaabb.h"
#include "cesphere.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_renditem ce_renditem;

typedef struct {
	void (*ctor)(ce_renditem* renditem, va_list args);
	void (*dtor)(ce_renditem* renditem);
	void (*render)(ce_renditem* renditem);
} ce_renditem_vtable;

struct ce_renditem {
	ce_aabb bounding_box;
	ce_sphere bounding_sphere;
	float dist2;
	bool transparent;
	ce_renditem_vtable vtable;
	size_t size;
	char impl[];
};

extern ce_renditem* ce_renditem_new(ce_renditem_vtable vtable, size_t size, ...);
extern void ce_renditem_del(ce_renditem* renditem);

extern void ce_renditem_render(ce_renditem* renditem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDITEM_H */
