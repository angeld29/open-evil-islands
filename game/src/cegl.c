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

// texture compression

#ifndef GL_VERSION_1_3
typedef void (APIENTRY *CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC)
				(GLenum target, GLint level, GLenum internal_format,
				GLsizei width, GLsizei height, GLint border,
				GLsizei image_size, const GLvoid* data);

static CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC ce_gl_compressed_tex_image_2d_proc;
#endif

// VBO

typedef void (APIENTRY *CE_GL_BIND_BUFFER_PROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY *CE_GL_DELETE_BUFFERS_PROC)
				(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY *CE_GL_GEN_BUFFERS_PROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY *CE_GL_BUFFER_DATA_PROC)
				(GLenum target, GLsizeiptr size,
				const GLvoid* data, GLenum usage);
typedef void (APIENTRY *CE_GL_BUFFER_SUB_DATA_PROC)
				(GLenum target, GLintptr offset,
				GLsizeiptr size, const GLvoid* data);

static CE_GL_BIND_BUFFER_PROC ce_gl_bind_buffer_proc;
static CE_GL_DELETE_BUFFERS_PROC ce_gl_delete_buffers_proc;
static CE_GL_GEN_BUFFERS_PROC ce_gl_gen_buffers_proc;
static CE_GL_BUFFER_DATA_PROC ce_gl_buffer_data_proc;
static CE_GL_BUFFER_SUB_DATA_PROC ce_gl_buffer_sub_data_proc;

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

static bool ce_gl_check_extension(const char* name)
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

bool ce_gl_init(void)
{
	assert(!ce_gl_inst.inited && "The gl subsystem has already been inited");
	ce_gl_inst.inited = true;

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO] =
		ce_gl_check_extension("GL_ARB_texture_non_power_of_two");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_RECTANGLE] =
		ce_gl_check_extension("GL_ARB_texture_rectangle") ||
		ce_gl_check_extension("GL_EXT_texture_rectangle") ||
		ce_gl_check_extension("GL_NV_texture_rectangle");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] =
		ce_gl_check_extension("GL_ARB_texture_compression");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
		ce_gl_check_extension("GL_EXT_texture_compression_s3tc");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] =
		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
		ce_gl_check_extension("GL_EXT_texture_compression_dxt1");

#ifndef GL_VERSION_1_3
	if (ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION]) {
		ce_gl_compressed_tex_image_2d_proc =
			(CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC)
				ce_gl_get_proc_address("glCompressedTexImage2DARB");
		if (NULL == ce_gl_compressed_tex_image_2d_proc) {
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] = false;
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] = false;
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] = false;
		}
	}
#endif

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_LOD] =
		ce_gl_check_extension("GL_SGIS_texture_lod") ||
		ce_gl_check_extension("GL_EXT_texture_lod");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_EDGE_CLAMP] =
		ce_gl_check_extension("GL_SGIS_texture_edge_clamp") ||
		ce_gl_check_extension("GL_EXT_texture_edge_clamp");

	ce_gl_inst.features[CE_GL_FEATURE_PACKED_PIXELS] =
		ce_gl_check_extension("GL_EXT_packed_pixels");

	ce_gl_inst.features[CE_GL_FEATURE_GENERATE_MIPMAP] =
		ce_gl_check_extension("GL_SGIS_generate_mipmap");

	ce_gl_inst.features[CE_GL_VERTEX_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_vertex_buffer_object");

	if (ce_gl_inst.features[CE_GL_VERTEX_BUFFER_OBJECT]) {
		ce_gl_bind_buffer_proc = (CE_GL_BIND_BUFFER_PROC)
								ce_gl_get_proc_address("glBindBufferARB");
		ce_gl_delete_buffers_proc = (CE_GL_DELETE_BUFFERS_PROC)
								ce_gl_get_proc_address("glDeleteBuffersARB");
		ce_gl_gen_buffers_proc = (CE_GL_GEN_BUFFERS_PROC)
								ce_gl_get_proc_address("glGenBuffersARB");
		ce_gl_buffer_data_proc = (CE_GL_BUFFER_DATA_PROC)
								ce_gl_get_proc_address("glBufferDataARB");
		ce_gl_buffer_sub_data_proc = (CE_GL_BUFFER_SUB_DATA_PROC)
								ce_gl_get_proc_address("glBufferSubDataARB");
		ce_gl_inst.features[CE_GL_VERTEX_BUFFER_OBJECT] =
			NULL != ce_gl_bind_buffer_proc &&
			NULL != ce_gl_delete_buffers_proc &&
			NULL != ce_gl_gen_buffers_proc &&
			NULL != ce_gl_buffer_data_proc &&
			NULL != ce_gl_buffer_sub_data_proc;
	}

	ce_gl_inst.features[CE_GL_WINDOW_POS] =
		ce_gl_check_extension("GL_ARB_window_pos") ||
		ce_gl_check_extension("GL_MESA_window_pos");

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
	assert(NULL != ce_gl_compressed_tex_image_2d_proc);
	(*ce_gl_compressed_tex_image_2d_proc)(target, level, internal_format,
									width, height, border, image_size, data);
#endif
}

void ce_gl_bind_buffer(GLenum target, GLuint buffer)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_bind_buffer_proc);
	(*ce_gl_bind_buffer_proc)(target, buffer);
}

void ce_gl_delete_buffers(GLsizei n, const GLuint* buffers)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_delete_buffers_proc);
	(*ce_gl_delete_buffers_proc)(n, buffers);
}

void ce_gl_gen_buffers(GLsizei n, GLuint* buffers)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_gen_buffers_proc);
	(*ce_gl_gen_buffers_proc)(n, buffers);
}

void ce_gl_buffer_data(GLenum target, GLsizeiptr size,
						const GLvoid* data, GLenum usage)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_buffer_data_proc);
	(*ce_gl_buffer_data_proc)(target, size, data, usage);
}

void ce_gl_buffer_sub_data(GLenum target, GLintptr offset,
							GLsizeiptr size, const GLvoid* data)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_buffer_sub_data_proc);
	(*ce_gl_buffer_sub_data_proc)(target, offset, size, data);
}
