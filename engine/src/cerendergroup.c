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
		ce_vector_for_each(rendergroup->renderlayers,
							(ce_vector_func1)ce_renderlayer_del);
		ce_vector_del(rendergroup->renderlayers);
		ce_free(rendergroup, sizeof(ce_rendergroup));
	}
}

void ce_rendergroup_clear(ce_rendergroup* rendergroup)
{
	ce_vector_for_each(rendergroup->renderlayers,
						(ce_vector_func1)ce_renderlayer_clear);
}

void ce_rendergroup_add(ce_rendergroup* rendergroup,
						ce_texture* texture,
						ce_renderitem* renderitem)
{
	for (int i = 0; i < rendergroup->renderlayers->count; ++i) {
		ce_renderlayer* renderlayer = rendergroup->renderlayers->items[i];
		if (ce_texture_equal(texture, renderlayer->texture)) {
			ce_renderlayer_add(renderlayer, renderitem);
			return;
		}
	}

	ce_renderlayer* renderlayer = ce_renderlayer_new(texture);
	ce_vector_push_back(rendergroup->renderlayers, renderlayer);

	ce_renderlayer_add(renderlayer, renderitem);
}

void ce_rendergroup_render(ce_rendergroup* rendergroup,
							ce_rendersystem* rendersystem)
{
	// FIXME: check ce_vector_empty, material may be wrong
	ce_rendersystem_apply_material(rendersystem, rendergroup->material);
	for (int i = 0; i < rendergroup->renderlayers->count; ++i) {
		ce_renderlayer_render(rendergroup->renderlayers->items[i], rendersystem);
	}
	ce_rendersystem_discard_material(rendersystem, rendergroup->material);
}
