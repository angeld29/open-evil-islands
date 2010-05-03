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

#include <assert.h>

#include <GL/glew.h>

#include "cealloc.h"
#include "ceocclusion.h"

struct ce_occlusion {
	bool supported;
	GLenum target;
	GLuint query;
	GLint result;
};

ce_occlusion* ce_occlusion_new(void)
{
	ce_occlusion* occlusion = ce_alloc(sizeof(ce_occlusion));
	occlusion->supported = GLEW_VERSION_1_5;
	occlusion->target = GL_SAMPLES_PASSED;

	// the first real result may be only in next 1-2 frames, so force to true
	occlusion->result = 1;

	if (occlusion->supported) {
		if (GLEW_VERSION_3_3 || GLEW_ARB_occlusion_query2) {
			occlusion->target = GL_ANY_SAMPLES_PASSED;
		}

		// check to make sure functionality is supported
		GLint result;
		glGetQueryiv(occlusion->target, GL_QUERY_COUNTER_BITS, &result);

		occlusion->supported = 0 != result;
	}

	if (occlusion->supported) {
		glGenQueries(1, &occlusion->query);
	}

	return occlusion;
}

void ce_occlusion_del(ce_occlusion* occlusion)
{
	if (NULL != occlusion) {
		if (occlusion->supported) {
			glDeleteQueries(1, &occlusion->query);
		}
		ce_free(occlusion, sizeof(ce_occlusion));
	}
}

bool ce_occlusion_query(ce_occlusion* occlusion,
	const ce_bbox* bbox, ce_rendersystem* rendersystem)
{
	if (!occlusion->supported) {
		return true;
	}

	// begin new query if and only if old one was finished
	// otherwise use result from last successful query
	GLint result = 1;

	if (glIsQuery(occlusion->query)) {
		glGetQueryObjectiv(occlusion->query,
			GL_QUERY_RESULT_AVAILABLE, &result);

		if (0 != result) {
			glGetQueryObjectiv(occlusion->query,
				GL_QUERY_RESULT, &occlusion->result);
		}
	}

	if (0 != result) {
		glBeginQuery(occlusion->target, occlusion->query);
		ce_rendersystem_apply_transform(rendersystem,
			&bbox->aabb.origin, &bbox->axis, &bbox->aabb.extents);
		ce_rendersystem_draw_solid_cube(rendersystem);
		ce_rendersystem_discard_transform(rendersystem);
		glEndQuery(occlusion->target);
	}

	return 0 != occlusion->result;
}
