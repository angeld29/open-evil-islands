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

#include "celogging.h"
#include "cealloc.h"
#include "cerenditem.h"

ce_renditem* ce_renditem_new(ce_renditem_vtable vtable, size_t size, ...)
{
	ce_renditem* renditem = ce_alloc_zero(sizeof(ce_renditem) + size);
	if (NULL == renditem) {
		ce_logging_error("renditem: could not allocate memory");
		return NULL;
	}

	renditem->vtable = vtable;
	renditem->size = size;

	if (NULL != renditem->vtable.ctor) {
		va_list args;
		va_start(args, size);
		(renditem->vtable.ctor)(renditem, args);
		va_end(args);
	}

	return renditem;
}

void ce_renditem_del(ce_renditem* renditem)
{
	if (NULL != renditem) {
		if (NULL != renditem->vtable.dtor) {
			(renditem->vtable.dtor)(renditem);
		}
		ce_free(renditem, sizeof(ce_renditem) + renditem->size);
	}
}

void ce_renditem_render(ce_renditem* renditem)
{
	(renditem->vtable.render)(renditem);
}
