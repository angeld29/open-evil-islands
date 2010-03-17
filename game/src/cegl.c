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

#include <string.h>
#include <assert.h>

#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wingdi.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif defined(__APPLE__) || defined(__APPLE_CC__)
#error Not implemented
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif

#ifndef GL_TEXTURE_MAX_LEVEL_SGIS
#define GL_TEXTURE_MAX_LEVEL_SGIS 0x813D
#endif

#ifndef GL_CLAMP_TO_EDGE_SGIS
#define GL_CLAMP_TO_EDGE_SGIS 0x812F
#endif

#ifndef GL_UNSIGNED_SHORT_4_4_4_4_EXT
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT 0x8033
#endif

#ifndef GL_UNSIGNED_SHORT_5_5_5_1_EXT
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT 0x8034
#endif

#ifndef GL_UNSIGNED_INT_8_8_8_8_EXT
#define GL_UNSIGNED_INT_8_8_8_8_EXT 0x8035
#endif

#ifndef GL_GENERATE_MIPMAP_SGIS
#define GL_GENERATE_MIPMAP_SGIS 0x8191
#endif

#ifndef GL_ARRAY_BUFFER_ARB
#define GL_ARRAY_BUFFER_ARB 0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER_ARB
#define GL_ELEMENT_ARRAY_BUFFER_ARB 0x8893
#endif

#ifndef GL_STREAM_DRAW_ARB
#define GL_STREAM_DRAW_ARB 0x88E0
#endif

#ifndef GL_STREAM_READ_ARB
#define GL_STREAM_READ_ARB 0x88E1
#endif

#ifndef GL_STREAM_COPY_ARB
#define GL_STREAM_COPY_ARB 0x88E2
#endif

#ifndef GL_STATIC_DRAW_ARB
#define GL_STATIC_DRAW_ARB 0x88E4
#endif

#ifndef GL_STATIC_READ_ARB
#define GL_STATIC_READ_ARB 0x88E5
#endif

#ifndef GL_STATIC_COPY_ARB
#define GL_STATIC_COPY_ARB 0x88E6
#endif

#ifndef GL_DYNAMIC_DRAW_ARB
#define GL_DYNAMIC_DRAW_ARB 0x88E8
#endif

#ifndef GL_DYNAMIC_READ_ARB
#define GL_DYNAMIC_READ_ARB 0x88E9
#endif

#ifndef GL_DYNAMIC_COPY_ARB
#define GL_DYNAMIC_COPY_ARB 0x88EA
#endif

#include "celogging.h"
#include "cegl.h"

const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
const GLenum CE_GL_TEXTURE_MAX_LEVEL = GL_TEXTURE_MAX_LEVEL_SGIS;
const GLenum CE_GL_CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE_SGIS;
const GLenum CE_GL_UNSIGNED_SHORT_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4_EXT;
const GLenum CE_GL_UNSIGNED_SHORT_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1_EXT;
const GLenum CE_GL_UNSIGNED_INT_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8_EXT;
const GLenum CE_GL_GENERATE_MIPMAP = GL_GENERATE_MIPMAP_SGIS;
const GLenum CE_GL_ARRAY_BUFFER = GL_ARRAY_BUFFER_ARB;
const GLenum CE_GL_ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER_ARB;
const GLenum CE_GL_STREAM_DRAW = GL_STREAM_DRAW_ARB;
const GLenum CE_GL_STREAM_READ = GL_STREAM_READ_ARB;
const GLenum CE_GL_STREAM_COPY = GL_STREAM_COPY_ARB;
const GLenum CE_GL_STATIC_DRAW = GL_STATIC_DRAW_ARB;
const GLenum CE_GL_STATIC_READ = GL_STATIC_READ_ARB;
const GLenum CE_GL_STATIC_COPY = GL_STATIC_COPY_ARB;
const GLenum CE_GL_DYNAMIC_DRAW = GL_DYNAMIC_DRAW_ARB;
const GLenum CE_GL_DYNAMIC_READ = GL_DYNAMIC_READ_ARB;
const GLenum CE_GL_DYNAMIC_COPY = GL_DYNAMIC_COPY_ARB;

#ifndef GL_VERSION_1_3
typedef void (APIENTRY *CE_PFNGLCOMPRESSEDTEXIMAGE2DPROC)
				(GLenum target, GLint level, GLenum internal_format,
				GLsizei width, GLsizei height, GLint border,
				GLsizei image_size, const GLvoid* data);

static CE_PFNGLCOMPRESSEDTEXIMAGE2DPROC ce_pfnglcompressedteximage2dproc;
#endif

typedef void (*ce_gl_ext_func_ptr)(void);

static ce_gl_ext_func_ptr ce_gl_get_proc_address(const char* name)
{
#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
	return (ce_gl_ext_func_ptr)wglGetProcAddress(name);
#elif defined(__APPLE__) || defined(__APPLE_CC__)
#error Not implemented
#else
#ifdef GLX_VERSION_1_4
	return (ce_gl_ext_func_ptr)glXGetProcAddress((const GLubyte*)name);
#endif
#endif
	return NULL;
}

static struct {
	bool inited;
	bool features[CE_GL_FEATURE_COUNT];
	const char* feature_names[CE_GL_FEATURE_COUNT];
} ce_gl_inst = {
	.feature_names = {
		"texture non power of two",
		"texture rectangle",
		"texture compression",
		"texture compression s3tc",
		"texture compression dxt1",
		"texture lod",
		"texture edge clamp",
		"packed pixels",
		"generate mipmap",
		"vertex buffer object",
		"window pos"
	}
};

static bool check_extension(const char* name)
{
#ifdef GLU_VERSION_1_3
	return gluCheckExtension((const GLubyte*)name, glGetString(GL_EXTENSIONS));
#else
	const size_t name_length = strlen(name);
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

	for (;;) {
		const char* where = strstr(extensions, name);

		if (NULL == where) {
			break;
		}

		const char* terminator = where + name_length;

		if ((extensions == where || ' ' == *(where - 1)) &&
				(' ' == *terminator || '\0' == *terminator)) {
			return true;
		}

		extensions = terminator;
	}

	return false;
#endif
}

bool ce_gl_init(void)
{
	assert(!ce_gl_inst.inited && "The gl subsystem has already been inited");
	ce_gl_inst.inited = true;

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO] =
		check_extension("GL_ARB_texture_non_power_of_two");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_RECTANGLE] =
		check_extension("GL_ARB_texture_rectangle") ||
		check_extension("GL_EXT_texture_rectangle") ||
		check_extension("GL_NV_texture_rectangle");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] =
		check_extension("GL_ARB_texture_compression");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
		check_extension("GL_EXT_texture_compression_s3tc");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] =
		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
		check_extension("GL_EXT_texture_compression_dxt1");

#ifndef GL_VERSION_1_3
	if (ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION]) {
		ce_pfnglcompressedteximage2dproc =
			(CE_PFNGLCOMPRESSEDTEXIMAGE2DPROC)
				ce_gl_get_proc_address("glCompressedTexImage2DARB");
		if (NULL == ce_pfnglcompressedteximage2dproc) {
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] = false;
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] = false;
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] = false;
		}
	}
#endif

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_LOD] =
		check_extension("GL_SGIS_texture_lod") ||
		check_extension("GL_EXT_texture_lod");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_EDGE_CLAMP] =
		check_extension("GL_SGIS_texture_edge_clamp") ||
		check_extension("GL_EXT_texture_edge_clamp");

	ce_gl_inst.features[CE_GL_FEATURE_PACKED_PIXELS] =
		check_extension("GL_EXT_packed_pixels");

	ce_gl_inst.features[CE_GL_FEATURE_GENERATE_MIPMAP] =
		check_extension("GL_SGIS_generate_mipmap");

	ce_gl_inst.features[CE_GL_VERTEX_BUFFER_OBJECT] =
		check_extension("GL_ARB_vertex_buffer_object");

	if (ce_gl_inst.features[CE_GL_VERTEX_BUFFER_OBJECT]) {
	}

	ce_gl_inst.features[CE_GL_WINDOW_POS] =
		check_extension("GL_ARB_window_pos") ||
		check_extension("GL_MESA_window_pos");

	for (int i = 0; i < CE_GL_FEATURE_COUNT; ++i) {
		ce_logging_write("opengl: checking for '%s' extension... %s",
							ce_gl_inst.feature_names[i],
							ce_gl_inst.features[i] ? "yes" : "no");
	}

	return true;
}

void ce_gl_term(void)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	ce_gl_inst.inited = false;
}

bool ce_gl_report_errors(void)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");

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
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	return ce_gl_inst.features[feature];
}

void ce_gl_compressed_tex_image_2d(GLenum target, GLint level,
									GLenum internal_format,
									GLsizei width, GLsizei height,
									GLint border, GLsizei image_size,
									const GLvoid* data)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
#ifdef GL_VERSION_1_3
	glCompressedTexImage2D(target, level, internal_format,
							width, height, border, image_size, data);
#else
	assert(NULL != ce_pfnglcompressedteximage2dproc);
	(*ce_pfnglcompressedteximage2dproc)(target, level, internal_format,
									width, height, border, image_size, data);
#endif
}
