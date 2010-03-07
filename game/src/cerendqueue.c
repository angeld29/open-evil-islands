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
#include "cerendqueue.h"

typedef int (*ce_rendqueue_comp)(const void* lhs, const void* rhs);

static int ce_rendqueue_comp_less(const void* lhs, const void* rhs)
{
	const ce_renditem* renditem1 = *(const ce_renditem**)lhs;
	const ce_renditem* renditem2 = *(const ce_renditem**)rhs;
	return ce_fisequal(renditem1->dist2, renditem2->dist2, CE_EPS_E3) ?
		0 : (renditem1->dist2 < renditem2->dist2 ? -1 : 1);
}

static int ce_rendqueue_comp_greater(const void* lhs, const void* rhs)
{
	const ce_renditem* renditem1 = *(const ce_renditem**)lhs;
	const ce_renditem* renditem2 = *(const ce_renditem**)rhs;
	return ce_fisequal(renditem1->dist2, renditem2->dist2, CE_EPS_E3) ?
		0 : (renditem1->dist2 > renditem2->dist2 ? -1 : 1);
}

static void ce_rendqueue_render_do(ce_vector* renditems,
									ce_rendqueue_comp comp)
{
	if (!ce_vector_empty(renditems)) {
		qsort(ce_vector_data(renditems),
				ce_vector_count(renditems),
				sizeof(ce_renditem*), comp);

		for (int i = 0, n = ce_vector_count(renditems); i < n; ++i) {
			ce_renditem_render(ce_vector_at(renditems, i));
		}
	}
}

ce_rendqueue* ce_rendqueue_new(void)
{
	ce_rendqueue* rendqueue = ce_alloc_zero(sizeof(ce_rendqueue));
	if (NULL == rendqueue) {
		ce_logging_error("rendqueue: could not allocate memory");
		return NULL;
	}

	if (NULL == (rendqueue->opacity_renditems = ce_vector_new()) ||
			NULL == (rendqueue->transparent_renditems = ce_vector_new())) {
		ce_rendqueue_del(rendqueue);
		return NULL;
	}

	return rendqueue;
}

void ce_rendqueue_del(ce_rendqueue* rendqueue)
{
	if (NULL != rendqueue) {
		ce_vector_del(rendqueue->transparent_renditems);
		ce_vector_del(rendqueue->opacity_renditems);
		ce_free(rendqueue, sizeof(ce_rendqueue));
	}
}

void ce_rendqueue_clear(ce_rendqueue* rendqueue)
{
	ce_vector_clear(rendqueue->opacity_renditems);
	ce_vector_clear(rendqueue->transparent_renditems);
}

void ce_rendqueue_render(ce_rendqueue* rendqueue)
{
	ce_rendqueue_render_do(rendqueue->opacity_renditems,
							ce_rendqueue_comp_less);
	ce_rendqueue_render_do(rendqueue->transparent_renditems,
							ce_rendqueue_comp_greater);
}

void ce_rendqueue_add_rendlayer(ce_rendqueue* rendqueue,
								const ce_rendlayer* rendlayer,
								const ce_vec3* eye,
								const ce_frustum* frustum)
{
	if (ce_frustum_test_box(frustum, &rendlayer->bounding_box)) {
		for (int i = 0, n = ce_vector_count(rendlayer->renditems); i < n; ++i) {
			ce_renditem* renditem = ce_vector_at(rendlayer->renditems, i);
			if (ce_frustum_test_box(frustum, &renditem->bounding_box)) {
				renditem->dist2 =
					ce_vec3_dist2(eye, &renditem->bounding_box.center);
				ce_vector_push_back(renditem->transparent ?
									rendqueue->transparent_renditems :
									rendqueue->opacity_renditems, renditem);
			}
		}
	}
}
