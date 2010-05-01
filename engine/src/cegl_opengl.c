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

#include "celogging.h"
#include "cegl.h"

static struct {
	bool inited;
} ce_gl_context;

bool ce_gl_init(void)
{
	assert(!ce_gl_context.inited && "The gl subsystem has already been inited");
	ce_gl_context.inited = true;

	ce_logging_write("opengl: %s", glGetString(GL_VENDOR));
	ce_logging_write("opengl: %s", glGetString(GL_RENDERER));
	ce_logging_write("opengl: using GL %s", glGetString(GL_VERSION));
	ce_logging_write("opengl: using GLU %s", gluGetString(GLU_VERSION));

	GLenum result = glewInit();
	if (GLEW_OK == result) {
		ce_logging_write("opengl: using GLEW %s", glewGetString(GLEW_VERSION));
	} else {
		ce_logging_critical("opengl: glewInit failed: %s", glewGetErrorString(result));
	}

	struct {
		const char* name;
		bool available;
	} extensions[] = {
		{ "ARB texture non power of two", GLEW_ARB_texture_non_power_of_two },
		{ "EXT texture compression s3tc", GLEW_EXT_texture_compression_s3tc },
		{ "EXT texture compression dxt1", GLEW_EXT_texture_compression_dxt1 },
		{ "ARB occlusion query 2", GLEW_ARB_occlusion_query2 },
		{ "AMD vertex shader tessellator", GLEW_AMD_vertex_shader_tessellator },
		{ "AMD performance monitor", GLEW_AMD_performance_monitor },
		{ "AMD meminfo", GLEW_ATI_meminfo },
	};

	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
		ce_logging_write("opengl: checking for '%s' extension... %s",
			extensions[i].name, extensions[i].available ? "yes" : "no");
	}

	return GLEW_OK == result;
}

void ce_gl_term(void)
{
	assert(ce_gl_context.inited && "The gl subsystem has not yet been inited");
	ce_gl_context.inited = false;
}

bool ce_gl_report_errors(void)
{
	assert(ce_gl_context.inited && "The gl subsystem has not yet been inited");

	bool reported = false;
	GLenum error;

	while (GL_NO_ERROR != (error = glGetError())) {
		ce_logging_error("opengl: error %u: %s", error, gluErrorString(error));
		reported = true;
	}

	return reported;
}
