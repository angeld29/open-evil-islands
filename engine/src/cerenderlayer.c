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

#include "cealloc.h"
#include "cerenderlayer.h"

ce_renderlayer* ce_renderlayer_new(ce_material* material)
{
	ce_renderlayer* renderlayer = ce_alloc(sizeof(ce_renderlayer));
	renderlayer->material = material;
	renderlayer->renderitems = ce_vector_new();
	return renderlayer;
}

void ce_renderlayer_del(ce_renderlayer* renderlayer)
{
	if (NULL != renderlayer) {
		ce_vector_for_each(renderlayer->renderitems,
							(ce_vector_func1)ce_renderitem_del);
		ce_vector_del(renderlayer->renderitems);
		ce_material_del(renderlayer->material);
		ce_free(renderlayer, sizeof(ce_renderlayer));
	}
}

void ce_renderlayer_add_renderitem(ce_renderlayer* renderlayer,
									ce_renderitem* renderitem)
{
	ce_vector_push_back(renderlayer->renderitems, renderitem);
}
