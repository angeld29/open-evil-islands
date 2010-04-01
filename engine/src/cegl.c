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

#include <stdarg.h>
#include <string.h>
#include <assert.h>

#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
#define WIN32_LEAN_AND_MEAN
// stupid windows needs to include windows.h before gl.h
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

#include "celogging.h"
#include "cegl.h"

const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT1 = 0x83F1;
const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3 = 0x83F2;
const GLenum CE_GL_TEXTURE_MAX_LEVEL = 0x813D;
const GLenum CE_GL_CLAMP_TO_EDGE = 0x812F;
const GLenum CE_GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033;
const GLenum CE_GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034;
const GLenum CE_GL_UNSIGNED_INT_8_8_8_8 = 0x8035;
const GLenum CE_GL_GENERATE_MIPMAP = 0x8191;
const GLenum CE_GL_ARRAY_BUFFER = 0x8892;
const GLenum CE_GL_ELEMENT_ARRAY_BUFFER = 0x8893;
const GLenum CE_GL_STREAM_DRAW = 0x88E0;
const GLenum CE_GL_STREAM_READ = 0x88E1;
const GLenum CE_GL_STREAM_COPY = 0x88E2;
const GLenum CE_GL_STATIC_DRAW = 0x88E4;
const GLenum CE_GL_STATIC_READ = 0x88E5;
const GLenum CE_GL_STATIC_COPY = 0x88E6;
const GLenum CE_GL_DYNAMIC_DRAW = 0x88E8;
const GLenum CE_GL_DYNAMIC_READ = 0x88E9;
const GLenum CE_GL_DYNAMIC_COPY = 0x88EA;
const GLenum CE_GL_POINT_SIZE_MIN = 0x8126;
const GLenum CE_GL_POINT_SIZE_MAX = 0x8127;
const GLenum CE_GL_POINT_FADE_THRESHOLD_SIZE = 0x8128;
const GLenum CE_GL_POINT_DISTANCE_ATTENUATION = 0x8129;
const GLenum CE_GL_POINT_SPRITE = 0x8861;
const GLenum CE_GL_COORD_REPLACE = 0x8862;
const GLenum CE_GL_VBO_FREE_MEMORY = 0x87FB;
const GLenum CE_GL_TEXTURE_FREE_MEMORY = 0x87FC;
const GLenum CE_GL_RENDERBUFFER_FREE_MEMORY = 0x87FD;
const GLenum CE_GLX_SAMPLE_BUFFERS = 100000;
const GLenum CE_GLX_SAMPLES = 100001;
const GLenum CE_WGL_SAMPLE_BUFFERS = 0x2041;
const GLenum CE_WGL_SAMPLES = 0x2042;
const GLenum CE_GL_MULTISAMPLE = 0x809D;
const GLenum CE_GL_SAMPLE_ALPHA_TO_COVERAGE = 0x809E;
const GLenum CE_GL_SAMPLE_ALPHA_TO_ONE = 0x809F;
const GLenum CE_GL_SAMPLE_COVERAGE = 0x80A0;
const GLenum CE_GL_MULTISAMPLE_BIT = 0x20000000;
const GLenum CE_GL_SAMPLE_BUFFERS = 0x80A8;
const GLenum CE_GL_SAMPLES = 0x80A9;
const GLenum CE_GL_SAMPLE_COVERAGE_VALUE = 0x80AA;
const GLenum CE_GL_SAMPLE_COVERAGE_INVERT = 0x80AB;

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

// window pos

typedef void (APIENTRY *CE_GL_WINDOW_POS_2F_PROC)(GLfloat x, GLfloat y);
typedef void (APIENTRY *CE_GL_WINDOW_POS_2FV_PROC)(const GLfloat* v);
typedef void (APIENTRY *CE_GL_WINDOW_POS_2I_PROC)(GLint x, GLint y);
typedef void (APIENTRY *CE_GL_WINDOW_POS_2IV_PROC)(const GLint* v);

static CE_GL_WINDOW_POS_2F_PROC ce_gl_window_pos_2f_proc;
static CE_GL_WINDOW_POS_2FV_PROC ce_gl_window_pos_2fv_proc;
static CE_GL_WINDOW_POS_2I_PROC ce_gl_window_pos_2i_proc;
static CE_GL_WINDOW_POS_2IV_PROC ce_gl_window_pos_2iv_proc;

// point parameters

typedef void (APIENTRY *CE_GL_POINT_PARAMETER_F_PROC)
				(GLenum pname, GLfloat param);
typedef void (APIENTRY *CE_GL_POINT_PARAMETER_FV_PROC)
				(GLenum pname, const GLfloat* params);

static CE_GL_POINT_PARAMETER_F_PROC ce_gl_point_parameter_f_proc;
static CE_GL_POINT_PARAMETER_FV_PROC ce_gl_point_parameter_fv_proc;

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

static ce_gl_ext_func_ptr ce_gl_get_first_proc_address(int count, ...)
{
	ce_gl_ext_func_ptr func = NULL;
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count && NULL == func; ++i) {
		func = ce_gl_get_proc_address(va_arg(args, const char*));
	}
	va_end(args);
	return func;
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
		"window pos",
		"point parameters",
		"point sprite",
		"meminfo",
		"multisample"
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

	ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_vertex_buffer_object");

	if (ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT]) {
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
		ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT] =
			NULL != ce_gl_bind_buffer_proc &&
			NULL != ce_gl_delete_buffers_proc &&
			NULL != ce_gl_gen_buffers_proc &&
			NULL != ce_gl_buffer_data_proc &&
			NULL != ce_gl_buffer_sub_data_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_WINDOW_POS] =
		ce_gl_check_extension("GL_ARB_window_pos") ||
		ce_gl_check_extension("GL_MESA_window_pos");

	if (ce_gl_inst.features[CE_GL_FEATURE_WINDOW_POS]) {
		ce_gl_window_pos_2f_proc = (CE_GL_WINDOW_POS_2F_PROC)
			ce_gl_get_first_proc_address(2, "glWindowPos2fARB",
											"glWindowPos2fMESA");
		ce_gl_window_pos_2fv_proc = (CE_GL_WINDOW_POS_2FV_PROC)
			ce_gl_get_first_proc_address(2, "glWindowPos2fvARB",
											"glWindowPos2fvMESA");
		ce_gl_window_pos_2i_proc = (CE_GL_WINDOW_POS_2I_PROC)
			ce_gl_get_first_proc_address(2, "glWindowPos2iARB",
											"glWindowPos2iMESA");
		ce_gl_window_pos_2iv_proc = (CE_GL_WINDOW_POS_2IV_PROC)
			ce_gl_get_first_proc_address(2, "glWindowPos2ivARB",
											"glWindowPos2ivMESA");
		ce_gl_inst.features[CE_GL_FEATURE_WINDOW_POS] =
			NULL != ce_gl_window_pos_2f_proc &&
			NULL != ce_gl_window_pos_2fv_proc &&
			NULL != ce_gl_window_pos_2i_proc &&
			NULL != ce_gl_window_pos_2iv_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_POINT_PARAMETERS] =
		ce_gl_check_extension("GL_ARB_point_parameters") ||
		ce_gl_check_extension("GL_EXT_point_parameters") ||
		ce_gl_check_extension("GL_SGIS_point_parameters");

	if (ce_gl_inst.features[CE_GL_FEATURE_POINT_PARAMETERS]) {
		ce_gl_point_parameter_f_proc = (CE_GL_POINT_PARAMETER_F_PROC)
			ce_gl_get_first_proc_address(3, "glPointParameterfARB",
											"glPointParameterfEXT",
											"glPointParameterfSGIS");
		ce_gl_point_parameter_fv_proc = (CE_GL_POINT_PARAMETER_FV_PROC)
			ce_gl_get_first_proc_address(3, "glPointParameterfvARB",
											"glPointParameterfvEXT",
											"glPointParameterfvSGIS");
		ce_gl_inst.features[CE_GL_FEATURE_POINT_PARAMETERS] =
			NULL != ce_gl_point_parameter_f_proc &&
			NULL != ce_gl_point_parameter_fv_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_POINT_SPRITE] =
		ce_gl_check_extension("GL_ARB_point_sprite") ||
		ce_gl_check_extension("GL_NV_point_sprite");

	ce_gl_inst.features[CE_GL_FEATURE_MEMINFO] =
		ce_gl_check_extension("GL_ATI_meminfo");

	ce_gl_inst.features[CE_GL_FEATURE_MULTISAMPLE] =
		ce_gl_check_extension("GL_ARB_multisample") ||
		ce_gl_check_extension("GLX_ARB_multisample") ||
		ce_gl_check_extension("WGL_ARB_multisample") ||
		ce_gl_check_extension("GL_EXT_multisample") ||
		ce_gl_check_extension("WGL_EXT_multisample") ||
		ce_gl_check_extension("GL_SGIS_multisample") ||
		ce_gl_check_extension("GLX_SGIS_multisample");

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

void ce_gl_window_pos_2f(GLfloat x, GLfloat y)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_window_pos_2f_proc);
	(*ce_gl_window_pos_2f_proc)(x, y);
}

void ce_gl_window_pos_2fv(const GLfloat* v)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_window_pos_2fv_proc);
	(*ce_gl_window_pos_2fv_proc)(v);
}

void ce_gl_window_pos_2i(GLint x, GLint y)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_window_pos_2i_proc);
	(*ce_gl_window_pos_2i_proc)(x, y);
}

void ce_gl_window_pos_2iv(const GLint* v)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_window_pos_2iv_proc);
	(*ce_gl_window_pos_2iv_proc)(v);
}

void ce_gl_point_parameter_f(GLenum pname, GLfloat param)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_point_parameter_f_proc);
	(*ce_gl_point_parameter_f_proc)(pname, param);
}

void ce_gl_point_parameter_fv(GLenum pname, GLfloat* params)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_point_parameter_fv_proc);
	(*ce_gl_point_parameter_fv_proc)(pname, params);
}
