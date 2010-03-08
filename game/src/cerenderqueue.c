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

#include "cemath.h"
#include "celogging.h"
#include "cealloc.h"
#include "cerenderqueue.h"

typedef int (*ce_renderqueue_comp)(const void* lhs, const void* rhs);

static int ce_renderqueue_comp_less(const void* lhs, const void* rhs)
{
	const ce_scenenode* scenenode1 = *(const ce_scenenode**)lhs;
	const ce_scenenode* scenenode2 = *(const ce_scenenode**)rhs;
	return ce_fisequal(scenenode1->dist2, scenenode2->dist2, CE_EPS_E3) ?
		0 : (scenenode1->dist2 < scenenode2->dist2 ? -1 : 1);
}

static int ce_renderqueue_comp_greater(const void* lhs, const void* rhs)
{
	const ce_scenenode* scenenode1 = *(const ce_scenenode**)lhs;
	const ce_scenenode* scenenode2 = *(const ce_scenenode**)rhs;
	return ce_fisequal(scenenode1->dist2, scenenode2->dist2, CE_EPS_E3) ?
		0 : (scenenode1->dist2 > scenenode2->dist2 ? -1 : 1);
}

static void ce_renderqueue_render_do(ce_vector* scenenodes,
									ce_renderqueue_comp comp)
{
	if (!ce_vector_empty(scenenodes)) {
		qsort(ce_vector_data(scenenodes),
				ce_vector_count(scenenodes),
				sizeof(ce_scenenode*), comp);

		for (int i = 0, n = ce_vector_count(scenenodes); i < n; ++i) {
			ce_scenenode* scenenode = ce_vector_at(scenenodes, i);
			ce_scenenode_apply_transform(scenenode);
			ce_renderitem_render(scenenode->renderitem);
			ce_scenenode_discard_transform(scenenode);
		}
	}
}

ce_renderqueue* ce_renderqueue_new(void)
{
	ce_renderqueue* renderqueue = ce_alloc_zero(sizeof(ce_renderqueue));
	if (NULL == renderqueue) {
		ce_logging_error("renderqueue: could not allocate memory");
		return NULL;
	}

	if (NULL == (renderqueue->opacity_scenenodes = ce_vector_new()) ||
			NULL == (renderqueue->transparent_scenenodes = ce_vector_new())) {
		ce_renderqueue_del(renderqueue);
		return NULL;
	}

	return renderqueue;
}

void ce_renderqueue_del(ce_renderqueue* renderqueue)
{
	if (NULL != renderqueue) {
		ce_vector_del(renderqueue->transparent_scenenodes);
		ce_vector_del(renderqueue->opacity_scenenodes);
		ce_free(renderqueue, sizeof(ce_renderqueue));
	}
}

void ce_renderqueue_clear(ce_renderqueue* renderqueue)
{
	ce_vector_clear(renderqueue->opacity_scenenodes);
	ce_vector_clear(renderqueue->transparent_scenenodes);
}

void ce_renderqueue_render(ce_renderqueue* renderqueue)
{
	ce_renderqueue_render_do(renderqueue->opacity_scenenodes,
							ce_renderqueue_comp_less);
	ce_renderqueue_render_do(renderqueue->transparent_scenenodes,
							ce_renderqueue_comp_greater);
}

void ce_renderqueue_add(ce_renderqueue* renderqueue, ce_scenenode* scenenode,
						const ce_vec3* eye, const ce_frustum* frustum)
{
	if (NULL != scenenode->renderitem &&
			ce_frustum_test_box(frustum, &scenenode->world_bounding_box)) {
		scenenode->dist2 = ce_vec3_dist2(eye, &scenenode->world_position);
		ce_vector_push_back(scenenode->renderitem->transparent ?
							renderqueue->transparent_scenenodes :
							renderqueue->opacity_scenenodes, scenenode);
	}
}

void ce_renderqueue_add_cascade(ce_renderqueue* renderqueue,
								ce_scenenode* scenenode,
								const ce_vec3* eye,
								const ce_frustum* frustum)
{
	ce_renderqueue_add(renderqueue, scenenode, eye, frustum);
	for (int i = 0, n = ce_vector_count(scenenode->child_scenenodes); i < n; ++i) {
		ce_renderqueue_add_cascade(renderqueue,
			ce_vector_at(scenenode->child_scenenodes, i), eye, frustum);
	}
}
