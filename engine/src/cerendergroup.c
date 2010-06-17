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
#include "cerendersystem.h"
#include "cerendergroup.h"

ce_rendergroup* ce_rendergroup_new(int priority, ce_material* material)
{
	ce_rendergroup* rendergroup = ce_alloc(sizeof(ce_rendergroup));
	rendergroup->priority = priority;
	rendergroup->material = material;
	rendergroup->renderlayers = ce_vector_new();
	return rendergroup;
}

void ce_rendergroup_del(ce_rendergroup* rendergroup)
{
	if (NULL != rendergroup) {
		ce_vector_for_each(rendergroup->renderlayers, ce_renderlayer_del);
		ce_vector_del(rendergroup->renderlayers);
		ce_free(rendergroup, sizeof(ce_rendergroup));
	}
}

void ce_rendergroup_clear(ce_rendergroup* rendergroup)
{
	ce_vector_for_each(rendergroup->renderlayers, ce_renderlayer_clear);
}

ce_renderlayer* ce_rendergroup_get(ce_rendergroup* rendergroup,
									ce_texture* texture)
{
	for (size_t i = 0; i < rendergroup->renderlayers->count; ++i) {
		ce_renderlayer* renderlayer = rendergroup->renderlayers->items[i];
		if (ce_texture_is_equal(texture, renderlayer->texture)) {
			return renderlayer;
		}
	}

	ce_renderlayer* renderlayer = ce_renderlayer_new(texture);
	ce_vector_push_back(rendergroup->renderlayers, renderlayer);

	return renderlayer;
}

void ce_rendergroup_render(ce_rendergroup* rendergroup)
{
	bool empty = true;
	for (size_t i = 0; i < rendergroup->renderlayers->count; ++i) {
		ce_renderlayer* renderlayer = rendergroup->renderlayers->items[i];
		empty = empty && ce_vector_empty(renderlayer->renderitems);
	}
	if (!empty) {
		ce_render_system_apply_material(rendergroup->material);
		ce_vector_for_each(rendergroup->renderlayers, ce_renderlayer_render);
		ce_render_system_discard_material();
	}
}
