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

	ce_logging_write("gl: vendor %s", glGetString(GL_VENDOR));
	ce_logging_write("gl: renderer %s", glGetString(GL_RENDERER));
	ce_logging_write("gl: using GL %s", glGetString(GL_VERSION));
	ce_logging_write("gl: using GLU %s", gluGetString(GLU_VERSION));

	GLenum result = glewInit();
	if (GLEW_OK == result) {
		ce_logging_write("gl: using GLEW %s", glewGetString(GLEW_VERSION));
	} else {
		ce_logging_critical("gl: glewInit failed: %s", glewGetErrorString(result));
	}

	struct {
		const char* name;
		bool available;
	} extensions[] = {
		{ "multitexture", GLEW_ARB_multitexture },
		{ "texture non power of two", GLEW_ARB_texture_non_power_of_two },
		{ "texture compression", GLEW_ARB_texture_compression },
		{ "texture compression s3tc", GLEW_EXT_texture_compression_s3tc },
		{ "texture compression dxt1", GLEW_EXT_texture_compression_dxt1 },
		{ "texture lod", GLEW_SGIS_texture_lod },
		{ "texture border clamp", GLEW_ARB_texture_border_clamp ||
									GLEW_SGIS_texture_border_clamp },
		{ "texture edge clamp", GLEW_EXT_texture_edge_clamp ||
								GLEW_SGIS_texture_edge_clamp },
		{ "texture float", GLEW_ARB_texture_float },
		{ "abgr", GLEW_EXT_abgr },
		{ "bgra", GLEW_EXT_bgra },
		{ "packed pixels", GLEW_EXT_packed_pixels },
		{ "generate mipmap", GLEW_SGIS_generate_mipmap },
		{ "window pos", GLEW_ARB_window_pos },
		{ "point parameters", GLEW_ARB_point_parameters },
		{ "point sprite", GLEW_ARB_point_sprite || GLEW_NV_point_sprite },
		{ "occlusion query", GLEW_ARB_occlusion_query },
		{ "occlusion query2", GLEW_ARB_occlusion_query2 },
		{ "multisample", GLEW_ARB_multisample },
		{ "vertex buffer object", GLEW_ARB_vertex_buffer_object },
		{ "copy buffer", GLEW_ARB_copy_buffer },
		{ "map buffer range", GLEW_ARB_map_buffer_range },
		{ "frame buffer object", GLEW_ARB_framebuffer_object },
		{ "pixel buffer object", GLEW_ARB_pixel_buffer_object ||
								GLEW_EXT_pixel_buffer_object },
		{ "texture buffer object", GLEW_ARB_texture_buffer_object ||
									GLEW_EXT_texture_buffer_object },
		{ "shading language 100", GLEW_ARB_shading_language_100 },
		{ "shader object", GLEW_ARB_shader_objects },
		{ "vertex shader", GLEW_ARB_vertex_shader },
		{ "fragment shader", GLEW_ARB_fragment_shader },
		{ "vertex shader tessellator", GLEW_AMD_vertex_shader_tessellator },
		{ "performance monitor", GLEW_AMD_performance_monitor },
		{ "meminfo", GLEW_ATI_meminfo },
	};

	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
		ce_logging_write("gl: checking for '%s' extension... %s",
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
		ce_logging_error("gl: error %u: %s", error, gluErrorString(error));
		reported = true;
	}

	return reported;
}
