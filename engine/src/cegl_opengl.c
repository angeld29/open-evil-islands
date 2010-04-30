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
	bool features[CE_GL_FEATURE_COUNT];
	const char* feature_names[CE_GL_FEATURE_COUNT];
} ce_gl_context = {
	.feature_names = {
		"multitexture",
		"texture non power of two",
		"texture compression",
		"texture compression s3tc",
		"texture compression dxt1",
		"texture lod",
		"texture border clamp",
		"texture edge clamp",
		"texture float",
		"abgr",
		"bgra",
		"packed pixels",
		"generate mipmap",
		"window pos",
		"point parameters",
		"point sprite",
		"occlusion query",
		"occlusion query2",
		"multisample",
		"vertex buffer object",
		"copy buffer",
		"map buffer range",
		"frame buffer object",
		"pixel buffer object",
		"texture buffer object",
		"shading language 100",
		"shader object",
		"vertex shader",
		"fragment shader",
		"vertex shader tessellator",
		"performance monitor",
		"meminfo"
	}
};

bool ce_gl_init(void)
{
	assert(!ce_gl_context.inited && "The gl subsystem has already been inited");
	ce_gl_context.inited = true;

	GLenum result = glewInit();
	if (GLEW_OK != result) {
		ce_logging_critical("opengl: glewInit failed: %s", glewGetErrorString(result));
	} else {
		ce_logging_write("opengl: using GLEW %s", glewGetString(GLEW_VERSION));

		ce_gl_context.features[CE_GL_FEATURE_MULTITEXTURE] =
			GLEW_ARB_multitexture;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO] =
			GLEW_ARB_texture_non_power_of_two;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] =
			GLEW_ARB_texture_compression;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
			GLEW_EXT_texture_compression_s3tc;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] =
			GLEW_EXT_texture_compression_dxt1;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_LOD] =
			GLEW_SGIS_texture_lod || glewIsSupported("GL_EXT_texture_lod");

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_BORDER_CLAMP] =
			GLEW_ARB_texture_border_clamp || GLEW_SGIS_texture_border_clamp;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_EDGE_CLAMP] =
			GLEW_EXT_texture_edge_clamp || GLEW_SGIS_texture_edge_clamp;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_FLOAT] =
			GLEW_ARB_texture_float || GLEW_ATI_texture_float;

		ce_gl_context.features[CE_GL_FEATURE_ABGR] = GLEW_EXT_abgr;
		ce_gl_context.features[CE_GL_FEATURE_BGRA] = GLEW_EXT_bgra;

		ce_gl_context.features[CE_GL_FEATURE_PACKED_PIXELS] =
			GLEW_EXT_packed_pixels;

		ce_gl_context.features[CE_GL_FEATURE_GENERATE_MIPMAP] =
			GLEW_SGIS_generate_mipmap;

		ce_gl_context.features[CE_GL_FEATURE_WINDOW_POS] =
			GLEW_ARB_window_pos || GLEW_MESA_window_pos;

		ce_gl_context.features[CE_GL_FEATURE_POINT_PARAMETERS] =
			GLEW_ARB_point_parameters || GLEW_EXT_point_parameters ||
			glewIsSupported("GL_SGIS_point_parameters");

		ce_gl_context.features[CE_GL_FEATURE_POINT_SPRITE] =
			GLEW_ARB_point_sprite || GLEW_NV_point_sprite;

		ce_gl_context.features[CE_GL_FEATURE_OCCLUSION_QUERY] =
			GLEW_ARB_occlusion_query;

		ce_gl_context.features[CE_GL_FEATURE_OCCLUSION_QUERY2] =
			GLEW_ARB_occlusion_query2;

		ce_gl_context.features[CE_GL_FEATURE_MULTISAMPLE] =
			GLEW_ARB_multisample || GLEW_EXT_multisample || GLEW_SGIS_multisample;

		ce_gl_context.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT] =
			GLEW_ARB_vertex_buffer_object;

		ce_gl_context.features[CE_GL_FEATURE_COPY_BUFFER] =
			GLEW_ARB_copy_buffer || glewIsSupported("GL_EXT_copy_buffer");

		ce_gl_context.features[CE_GL_FEATURE_MAP_BUFFER_RANGE] =
			GLEW_ARB_map_buffer_range;

		ce_gl_context.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT] =
			GLEW_ARB_framebuffer_object ||
			(GLEW_EXT_framebuffer_object && GLEW_EXT_framebuffer_blit &&
			GLEW_EXT_framebuffer_multisample && GLEW_EXT_packed_depth_stencil);

		ce_gl_context.features[CE_GL_FEATURE_PIXEL_BUFFER_OBJECT] =
			GLEW_ARB_pixel_buffer_object || GLEW_EXT_pixel_buffer_object;

		ce_gl_context.features[CE_GL_FEATURE_TEXTURE_BUFFER_OBJECT] =
			GLEW_ARB_texture_buffer_object || GLEW_EXT_texture_buffer_object;

		ce_gl_context.features[CE_GL_FEATURE_SHADING_LANGUAGE_100] =
			GLEW_ARB_shading_language_100;

		ce_gl_context.features[CE_GL_FEATURE_SHADER_OBJECT] =
			GLEW_ARB_shader_objects;

		ce_gl_context.features[CE_GL_FEATURE_VERTEX_SHADER] =
			GLEW_ARB_vertex_shader;

		ce_gl_context.features[CE_GL_FEATURE_FRAGMENT_SHADER] =
			GLEW_ARB_fragment_shader;

		ce_gl_context.features[CE_GL_FEATURE_VERTEX_SHADER_TESSELLATOR] =
			GLEW_AMD_vertex_shader_tessellator;

		ce_gl_context.features[CE_GL_FEATURE_PERFORMANCE_MONITOR] =
			GLEW_AMD_performance_monitor;

		ce_gl_context.features[CE_GL_FEATURE_MEMINFO] = GLEW_ATI_meminfo;
	}

	for (int i = 0; i < CE_GL_FEATURE_COUNT; ++i) {
		ce_logging_write("opengl: checking for '%s' extension... %s",
							ce_gl_context.feature_names[i],
							ce_gl_context.features[i] ? "yes" : "no");
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

bool ce_gl_query_feature(ce_gl_feature feature)
{
	assert(ce_gl_context.inited && "The gl subsystem has not yet been inited");
	return ce_gl_context.features[feature];
}
