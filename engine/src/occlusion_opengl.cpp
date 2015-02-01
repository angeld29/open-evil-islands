/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "alloc.hpp"
#include "opengl.hpp"
#include "rendersystem.hpp"
#include "occlusion.hpp"

namespace cursedearth
{
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

bool ce_occlusion_query(ce_occlusion* occlusion, const bbox_t* bbox)
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
        glDisable(GL_CULL_FACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        glBeginQuery(occlusion->target, occlusion->query);

        ce_render_system_apply_transform(&bbox->aabb.origin,
                                        &bbox->axis, &bbox->aabb.extents);
        ce_render_system_draw_solid_cube();
        ce_render_system_discard_transform();

        glEndQuery(occlusion->target);

        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_CULL_FACE);
    }

    return 0 != occlusion->result;
}
}
