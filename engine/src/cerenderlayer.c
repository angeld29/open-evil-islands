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

ce_renderlayer* ce_renderlayer_new(ce_texture* texture)
{
	ce_renderlayer* renderlayer = ce_alloc(sizeof(ce_renderlayer));
	renderlayer->texture = ce_texture_add_ref(texture);
	renderlayer->renderitems = ce_vector_new();
	return renderlayer;
}

void ce_renderlayer_del(ce_renderlayer* renderlayer)
{
	if (NULL != renderlayer) {
		ce_vector_del(renderlayer->renderitems);
		ce_texture_del(renderlayer->texture);
		ce_free(renderlayer, sizeof(ce_renderlayer));
	}
}

void ce_renderlayer_clear(ce_renderlayer* renderlayer)
{
	ce_vector_clear(renderlayer->renderitems);
}

void ce_renderlayer_add(ce_renderlayer* renderlayer,
						ce_renderitem* renderitem)
{
	ce_vector_push_back(renderlayer->renderitems, renderitem);
}

void ce_renderlayer_render(ce_renderlayer* renderlayer,
							ce_rendersystem* rendersystem)
{
	ce_texture_bind(renderlayer->texture);
	for (int i = 0; i < renderlayer->renderitems->count; ++i) {
		ce_renderitem* renderitem = renderlayer->renderitems->items[i];
		ce_rendersystem_apply_transform(rendersystem,
										&renderitem->world_position,
										&renderitem->world_orientation,
										&CE_VEC3_UNIT_SCALE);
		ce_renderitem_render(renderitem);
		ce_rendersystem_discard_transform(rendersystem);
	}
	ce_texture_unbind(renderlayer->texture);
}
