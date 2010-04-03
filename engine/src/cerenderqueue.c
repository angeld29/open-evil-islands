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

#include <stdlib.h>
#include <assert.h>

#include "cemath.h"
#include "cealloc.h"
#include "cerenderqueue.h"

ce_renderqueue* ce_renderqueue_new(void)
{
	ce_renderqueue* renderqueue = ce_alloc(sizeof(ce_renderqueue));
	renderqueue->rendergroups = ce_vector_new();
	return renderqueue;
}

void ce_renderqueue_del(ce_renderqueue* renderqueue)
{
	if (NULL != renderqueue) {
		ce_vector_for_each(renderqueue->rendergroups,
							(ce_vector_func1)ce_rendergroup_del);
		ce_vector_del(renderqueue->rendergroups);
		ce_free(renderqueue, sizeof(ce_renderqueue));
	}
}

void ce_renderqueue_clear(ce_renderqueue* renderqueue)
{
	ce_vector_for_each(renderqueue->rendergroups,
						(ce_vector_func1)ce_rendergroup_clear);
}

static int ce_renderqueue_comp(const void* lhs, const void* rhs)
{
	return (*(const ce_rendergroup**)lhs)->priority -
			(*(const ce_rendergroup**)rhs)->priority;
}

void ce_renderqueue_add(ce_renderqueue* renderqueue,
						int priority, ce_material* material)
{
	for (int i = 0; i < renderqueue->rendergroups->count; ++i) {
		ce_rendergroup* rendergroup = renderqueue->rendergroups->items[i];
		if (priority == rendergroup->priority) {
			ce_rendergroup_del(rendergroup);
			ce_vector_remove_unordered(renderqueue->rendergroups, i);
			break;
		}
	}
	ce_vector_push_back(renderqueue->rendergroups,
		ce_rendergroup_new(priority, material));
	qsort(renderqueue->rendergroups->items,
		renderqueue->rendergroups->count,
		sizeof(ce_rendergroup*), ce_renderqueue_comp);
}

ce_rendergroup* ce_renderqueue_get(ce_renderqueue* renderqueue, int priority)
{
	for (int i = 0; i < renderqueue->rendergroups->count; ++i) {
		ce_rendergroup* rendergroup = renderqueue->rendergroups->items[i];
		if (priority == rendergroup->priority) {
			return rendergroup;
		}
	}
	return NULL;
}

void ce_renderqueue_render(ce_renderqueue* renderqueue,
							ce_rendersystem* rendersystem)
{
	for (int i = 0; i < renderqueue->rendergroups->count; ++i) {
		ce_rendergroup_render(renderqueue->rendergroups->items[i], rendersystem);
	}
}
