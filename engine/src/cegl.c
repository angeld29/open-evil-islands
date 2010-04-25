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

// texture compression
const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT1 = 0x83F1;
const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3 = 0x83F2;

typedef void (APIENTRY *CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC)
				(GLenum target, GLint level, GLenum internal_format,
				GLsizei width, GLsizei height, GLint border,
				GLsizei image_size, const GLvoid* data);

static CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC ce_gl_compressed_tex_image_2d_proc;

// texture lod
const GLenum CE_GL_TEXTURE_MAX_LEVEL = 0x813D;

// texture clamp
const GLenum CE_GL_CLAMP_TO_BORDER = 0x812D;
const GLenum CE_GL_CLAMP_TO_EDGE = 0x812F;

// texture float
const GLenum CE_GL_RGBA32F = 0x8814;
const GLenum CE_GL_RGB32F = 0x8815;
const GLenum CE_GL_ALPHA32F = 0x8816;
const GLenum CE_GL_RGBA16F = 0x881A;
const GLenum CE_GL_RGB16F = 0x881B;
const GLenum CE_GL_ALPHA16F = 0x881C;

// color formats
const GLenum CE_GL_ABGR = 0x8000;
const GLenum CE_GL_BGR = 0x80E0;
const GLenum CE_GL_BGRA = 0x80E1;

// packed pixels
const GLenum CE_GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033;
const GLenum CE_GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034;
const GLenum CE_GL_UNSIGNED_INT_8_8_8_8 = 0x8035;

// generate mipmap
const GLenum CE_GL_GENERATE_MIPMAP = 0x8191;

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
const GLenum CE_GL_POINT_SIZE_MIN = 0x8126;
const GLenum CE_GL_POINT_SIZE_MAX = 0x8127;
const GLenum CE_GL_POINT_FADE_THRESHOLD_SIZE = 0x8128;
const GLenum CE_GL_POINT_DISTANCE_ATTENUATION = 0x8129;

typedef void (APIENTRY *CE_GL_POINT_PARAMETER_F_PROC)
				(GLenum pname, GLfloat param);
typedef void (APIENTRY *CE_GL_POINT_PARAMETER_FV_PROC)
				(GLenum pname, const GLfloat* params);

static CE_GL_POINT_PARAMETER_F_PROC ce_gl_point_parameter_f_proc;
static CE_GL_POINT_PARAMETER_FV_PROC ce_gl_point_parameter_fv_proc;

// point sprite
const GLenum CE_GL_POINT_SPRITE = 0x8861;
const GLenum CE_GL_COORD_REPLACE = 0x8862;

// occlusion query
const GLenum CE_GL_SAMPLES_PASSED = 0x8914;
const GLenum CE_GL_ANY_SAMPLES_PASSED = 0x8C2F;
const GLenum CE_GL_QUERY_COUNTER_BITS = 0x8864;
const GLenum CE_GL_CURRENT_QUERY = 0x8865;
const GLenum CE_GL_QUERY_RESULT = 0x8866;
const GLenum CE_GL_QUERY_RESULT_AVAILABLE = 0x8867;

typedef void (APIENTRY *CE_GL_GEN_QUERIES_PROC)(GLsizei n, GLuint* ids);
typedef void (APIENTRY *CE_GL_DELETE_QUERIES_PROC)(GLsizei n, const GLuint* ids);
typedef GLboolean (APIENTRY *CE_GL_IS_QUERY_PROC)(GLuint id);
typedef void (APIENTRY *CE_GL_BEGIN_QUERY_PROC)(GLenum target, GLuint id);
typedef void (APIENTRY *CE_GL_END_QUERY_PROC)(GLenum target);
typedef void (APIENTRY *CE_GL_GET_QUERY_IV_PROC)
			(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *CE_GL_GET_QUERY_OBJECT_IV_PROC)
			(GLuint id, GLenum pname, GLint* params);
typedef void (APIENTRY *CE_GL_GET_QUERY_OBJECT_UIV_PROC)
			(GLuint id, GLenum pname, GLuint* params);

static CE_GL_GEN_QUERIES_PROC ce_gl_gen_queries_proc;
static CE_GL_DELETE_QUERIES_PROC ce_gl_delete_queries_proc;
static CE_GL_IS_QUERY_PROC ce_gl_is_query_proc;
static CE_GL_BEGIN_QUERY_PROC ce_gl_begin_query_proc;
static CE_GL_END_QUERY_PROC ce_gl_end_query_proc;
static CE_GL_GET_QUERY_IV_PROC ce_gl_get_query_iv_proc;
static CE_GL_GET_QUERY_OBJECT_IV_PROC ce_gl_get_query_object_iv_proc;
static CE_GL_GET_QUERY_OBJECT_UIV_PROC ce_gl_get_query_object_uiv_proc;

// multisample
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

// VBO
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
const GLenum CE_GL_READ_ONLY = 0x88B8;
const GLenum CE_GL_WRITE_ONLY = 0x88B9;
const GLenum CE_GL_READ_WRITE = 0x88BA;

typedef void (APIENTRY *CE_GL_BIND_BUFFER_PROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY *CE_GL_DELETE_BUFFERS_PROC)
				(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY *CE_GL_GEN_BUFFERS_PROC)(GLsizei n, GLuint* buffers);
typedef GLboolean (APIENTRY *CE_GL_IS_BUFFER_PROC)(GLuint buffer);
typedef void (APIENTRY *CE_GL_BUFFER_DATA_PROC)
				(GLenum target, GLsizeiptr size,
				const GLvoid* data, GLenum usage);
typedef void (APIENTRY *CE_GL_BUFFER_SUB_DATA_PROC)
				(GLenum target, GLintptr offset,
				GLsizeiptr size, const GLvoid* data);
typedef void (APIENTRY *CE_GL_GET_BUFFER_SUB_DATA_PROC)
				(GLenum target, GLintptr offset, GLsizeiptr size, void* data);
typedef void* (APIENTRY *CE_GL_MAP_BUFFER_PROC)(GLenum target, GLenum access);
typedef GLboolean (APIENTRY *CE_GL_UNMAP_BUFFER_PROC)(GLenum target);
typedef void (APIENTRY *CE_GL_GET_BUFFER_PARAMETER_IV_PROC)
				(GLenum target, GLenum pname, int* params);
typedef void (APIENTRY *CE_GL_GET_BUFFER_POINTER_V_PROC)
				(GLenum target, GLenum pname, void** params);

static CE_GL_BIND_BUFFER_PROC ce_gl_bind_buffer_proc;
static CE_GL_DELETE_BUFFERS_PROC ce_gl_delete_buffers_proc;
static CE_GL_GEN_BUFFERS_PROC ce_gl_gen_buffers_proc;
static CE_GL_IS_BUFFER_PROC ce_gl_is_buffer_proc;
static CE_GL_BUFFER_DATA_PROC ce_gl_buffer_data_proc;
static CE_GL_BUFFER_SUB_DATA_PROC ce_gl_buffer_sub_data_proc;
static CE_GL_GET_BUFFER_SUB_DATA_PROC ce_gl_get_buffer_sub_data_proc;
static CE_GL_MAP_BUFFER_PROC ce_gl_map_buffer_proc;
static CE_GL_UNMAP_BUFFER_PROC ce_gl_unmap_buffer_proc;
static CE_GL_GET_BUFFER_PARAMETER_IV_PROC ce_gl_get_buffer_parameter_iv_proc;
static CE_GL_GET_BUFFER_POINTER_V_PROC ce_gl_get_buffer_pointer_v_proc;

// FBO
const GLenum CE_GL_FRAME_BUFFER = 0x8D40;
const GLenum CE_GL_READ_FRAME_BUFFER = 0x8CA8;
const GLenum CE_GL_DRAW_FRAME_BUFFER = 0x8CA9;
const GLenum CE_GL_COLOR_ATTACHMENT0 = 0x8CE0;
const GLenum CE_GL_COLOR_ATTACHMENT1 = 0x8CE1;
const GLenum CE_GL_FRAME_BUFFER_COMPLETE = 0x8CD5;

typedef void (APIENTRY *CE_GL_BIND_FRAME_BUFFER_PROC)
				(GLenum target, GLuint buffer);
typedef void (APIENTRY *CE_GL_DELETE_FRAME_BUFFERS_PROC)
				(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY *CE_GL_GEN_FRAME_BUFFERS_PROC)
				(GLsizei n, GLuint* buffers);
typedef GLenum (APIENTRY *CE_GL_CHECK_FRAME_BUFFER_STATUS_PROC)(GLenum target);
typedef void (APIENTRY *CE_GL_FRAME_BUFFER_TEXTURE_2D_PROC)
				(GLenum target, GLenum attachment,
				GLenum tex_target, GLuint texture, GLint level);
typedef void (APIENTRY *CE_GL_GENERATE_MIPMAP_PROC)(GLenum target);

static CE_GL_BIND_FRAME_BUFFER_PROC ce_gl_bind_frame_buffer_proc;
static CE_GL_DELETE_FRAME_BUFFERS_PROC ce_gl_delete_frame_buffers_proc;
static CE_GL_GEN_FRAME_BUFFERS_PROC ce_gl_gen_frame_buffers_proc;
static CE_GL_CHECK_FRAME_BUFFER_STATUS_PROC ce_gl_check_frame_buffer_status_proc;
static CE_GL_FRAME_BUFFER_TEXTURE_2D_PROC ce_gl_frame_buffer_texture_2d_proc;
static CE_GL_GENERATE_MIPMAP_PROC ce_gl_generate_mipmap_proc;

// PBO
const GLenum CE_GL_PIXEL_PACK_BUFFER = 0x88EB;
const GLenum CE_GL_PIXEL_UNPACK_BUFFER = 0x88EC;

// TBO
const GLenum CE_GL_TEXTURE_BUFFER = 0x8C2A;
const GLenum CE_GL_MAX_TEXTURE_BUFFER_SIZE = 0x8C2B;
const GLenum CE_GL_TEXTURE_BINDING_BUFFER = 0x8C2C;
const GLenum CE_GL_TEXTURE_BUFFER_DATA_STORE_BINDING = 0x8C2D;
const GLenum CE_GL_TEXTURE_BUFFER_FORMAT = 0x8C2E;

typedef void (APIENTRY *CE_GL_TEX_BUFFER_PROC)
			(GLenum target, GLenum internal_format, GLuint buffer);

static CE_GL_TEX_BUFFER_PROC ce_gl_tex_buffer_proc;

// shading language
const GLenum CE_GL_SHADING_LANGUAGE_VERSION = 0x8B8C;

// shader object
const GLenum CE_GL_PROGRAM_OBJECT = 0x8B40;
const GLenum CE_GL_OBJECT_TYPE = 0x8B4E;
const GLenum CE_GL_OBJECT_SUBTYPE = 0x8B4F;
const GLenum CE_GL_OBJECT_DELETE_STATUS = 0x8B80;
const GLenum CE_GL_OBJECT_COMPILE_STATUS = 0x8B81;
const GLenum CE_GL_OBJECT_LINK_STATUS = 0x8B82;
const GLenum CE_GL_OBJECT_VALIDATE_STATUS = 0x8B83;
const GLenum CE_GL_OBJECT_INFO_LOG_LENGTH = 0x8B84;
const GLenum CE_GL_OBJECT_ATTACHED_OBJECTS = 0x8B85;
const GLenum CE_GL_OBJECT_ACTIVE_UNIFORMS = 0x8B86;
const GLenum CE_GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH = 0x8B87;
const GLenum CE_GL_OBJECT_SHADER_SOURCE_LENGTH = 0x8B88;
const GLenum CE_GL_SHADER_OBJECT = 0x8B48;

typedef void (APIENTRY *CE_GL_DELETE_OBJECT_PROC)(GLhandle object);
typedef GLhandle (APIENTRY *CE_GL_GET_HANDLE_PROC)(GLenum pname);
typedef void (APIENTRY *CE_GL_DETACH_OBJECT_PROC)
			(GLhandle container, GLhandle object);
typedef GLhandle (APIENTRY *CE_GL_CREATE_SHADER_OBJECT_PROC)(GLenum type);
typedef void (APIENTRY *CE_GL_SHADER_SOURCE_PROC)
			(GLhandle object, GLsizei count,
			const GLchar** string, const int* length);
typedef void (APIENTRY *CE_GL_COMPILE_SHADER_PROC)(GLhandle object);
typedef GLhandle (APIENTRY *CE_GL_CREATE_PROGRAM_OBJECT_PROC)(void);
typedef void (APIENTRY *CE_GL_ATTACH_OBJECT_PROC)
			(GLhandle container, GLhandle object);
typedef void (APIENTRY *CE_GL_LINK_PROGRAM_OBJECT_PROC)(GLhandle object);
typedef void (APIENTRY *CE_GL_USE_PROGRAM_OBJECT_PROC)(GLhandle object);
typedef void (APIENTRY *CE_GL_VALIDATE_PROGRAM_OBJECT_PROC)(GLhandle object);

static CE_GL_DELETE_OBJECT_PROC ce_gl_delete_object_proc;
static CE_GL_GET_HANDLE_PROC ce_gl_get_handle_proc;
static CE_GL_DETACH_OBJECT_PROC ce_gl_detach_object_proc;
static CE_GL_CREATE_SHADER_OBJECT_PROC ce_gl_create_shader_object_proc;
static CE_GL_SHADER_SOURCE_PROC ce_gl_shader_source_proc;
static CE_GL_COMPILE_SHADER_PROC ce_gl_compile_shader_proc;
static CE_GL_CREATE_PROGRAM_OBJECT_PROC ce_gl_create_program_object_proc;
static CE_GL_ATTACH_OBJECT_PROC ce_gl_attach_object_proc;
static CE_GL_LINK_PROGRAM_OBJECT_PROC ce_gl_link_program_object_proc;
static CE_GL_USE_PROGRAM_OBJECT_PROC ce_gl_use_program_object_proc;
static CE_GL_VALIDATE_PROGRAM_OBJECT_PROC ce_gl_validate_program_object_proc;

// vertex shader
const GLenum CE_GL_VERTEX_SHADER = 0x8B31;

// fragment shader
const GLenum CE_GL_FRAGMENT_SHADER = 0x8B30;

// vertex shader tessellator
const GLenum CE_GL_VST_SAMPLER_BUFFER = 0x9001;
const GLenum CE_GL_VST_INT_SAMPLER_BUFFER = 0x9002;
const GLenum CE_GL_VST_UNSIGNED_INT_SAMPLER_BUFFER = 0x9003;
const GLenum CE_GL_VST_DISCRETE = 0x9006;
const GLenum CE_GL_VST_CONTINUOUS = 0x9007;
const GLenum CE_GL_VST_TESSELLATION_MODE = 0x9004;
const GLenum CE_GL_VST_TESSELLATION_FACTOR = 0x9005;

typedef void (APIENTRY *CE_GL_VST_SET_TESSELLATION_FACTOR_PROC)(GLfloat factor);
typedef void (APIENTRY *CE_GL_VST_SET_TESSELLATION_MODE_PROC)(GLenum mode);

static CE_GL_VST_SET_TESSELLATION_FACTOR_PROC ce_gl_vst_set_tessellation_factor_proc;
static CE_GL_VST_SET_TESSELLATION_MODE_PROC ce_gl_vst_set_tessellation_mode_proc;

// performance monitor
const GLenum CE_GL_PERFMON_COUNTER_TYPE = 0x8BC0;
const GLenum CE_GL_PERFMON_COUNTER_RANGE = 0x8BC1;
const GLenum CE_GL_PERFMON_UNSIGNED_INT64 = 0x8BC2;
const GLenum CE_GL_PERFMON_PERCENTAGE = 0x8BC3;
const GLenum CE_GL_PERFMON_RESULT_AVAILABLE = 0x8BC4;
const GLenum CE_GL_PERFMON_RESULT_SIZE = 0x8BC5;
const GLenum CE_GL_PERFMON_RESULT = 0x8BC6;

typedef void (APIENTRY *CE_GL_GET_PERFMON_GROUPS_PROC)(GLint*, GLsizei, GLuint*);
typedef void (APIENTRY *CE_GL_GET_PERFMON_COUNTERS_PROC)
						(GLuint, GLint*, GLint*, GLsizei, GLuint*);
typedef void (APIENTRY *CE_GL_GET_PERFMON_GROUP_STR_PROC)
						(GLuint, GLsizei, GLsizei*, char*);
typedef void (APIENTRY *CE_GL_GET_PERFMON_COUNTER_STR_PROC)
						(GLuint, GLuint, GLsizei, GLsizei*, char*);
typedef void (APIENTRY *CE_GL_GET_PERFMON_COUNTER_INFO_PROC)
						(GLuint, GLuint, GLenum, GLvoid*);
typedef void (APIENTRY *CE_GL_GEN_PERFMONS_PROC)(GLsizei, GLuint*);
typedef void (APIENTRY *CE_GL_DELETE_PERFMONS_PROC)(GLsizei, GLuint*);
typedef void (APIENTRY *CE_GL_SELECT_PERFMON_COUNTERS_PROC)
						(GLuint, GLboolean, GLuint, GLint, GLuint*);
typedef void (APIENTRY *CE_GL_BEGIN_PERFMON_PROC)(GLuint);
typedef void (APIENTRY *CE_GL_END_PERFMON_PROC)(GLuint);
typedef void (APIENTRY *CE_GL_GET_PERFMON_COUNTER_DATA_PROC)
						(GLuint, GLenum, GLsizei, GLuint*, GLint*);

static CE_GL_GET_PERFMON_GROUPS_PROC ce_gl_get_perfmon_groups_proc;
static CE_GL_GET_PERFMON_COUNTERS_PROC ce_gl_get_perfmon_counters_proc;
static CE_GL_GET_PERFMON_GROUP_STR_PROC ce_gl_get_perfmon_group_str_proc;
static CE_GL_GET_PERFMON_COUNTER_STR_PROC ce_gl_get_perfmon_counter_str_proc;
static CE_GL_GET_PERFMON_COUNTER_INFO_PROC ce_gl_get_perfmon_counter_info_proc;
static CE_GL_GEN_PERFMONS_PROC ce_gl_gen_perfmons_proc;
static CE_GL_DELETE_PERFMONS_PROC ce_gl_delete_perfmons_proc;
static CE_GL_SELECT_PERFMON_COUNTERS_PROC ce_gl_select_perfmon_counters_proc;
static CE_GL_BEGIN_PERFMON_PROC ce_gl_begin_perfmon_proc;
static CE_GL_END_PERFMON_PROC ce_gl_end_perfmon_proc;
static CE_GL_GET_PERFMON_COUNTER_DATA_PROC ce_gl_get_perfmon_counter_data_proc;

// meminfo
const GLenum CE_GL_VBO_FREE_MEMORY = 0x87FB;
const GLenum CE_GL_TEXTURE_FREE_MEMORY = 0x87FC;
const GLenum CE_GL_RENDERBUFFER_FREE_MEMORY = 0x87FD;

// common API
typedef void (*ce_gl_ext_proc_ptr)(void);

static ce_gl_ext_proc_ptr ce_gl_get_proc_address(const char* name)
{
#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
	return (ce_gl_ext_proc_ptr)wglGetProcAddress(name);
#elif defined(__APPLE__) || defined(__APPLE_CC__)
#error Not implemented
#else
#ifdef GLX_VERSION_1_4
	return (ce_gl_ext_proc_ptr)glXGetProcAddress((const GLubyte*)name);
#endif
#endif
	return NULL;
}

static ce_gl_ext_proc_ptr ce_gl_get_first_proc_address(int count, ...)
{
	ce_gl_ext_proc_ptr func = NULL;
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

	if (ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION]) {
		ce_gl_compressed_tex_image_2d_proc =
			(CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC)
				ce_gl_get_proc_address("glCompressedTexImage2DARB");

		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] =
			NULL != ce_gl_compressed_tex_image_2d_proc;

		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
			ce_gl_check_extension("GL_EXT_texture_compression_s3tc");

		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] =
			ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
			ce_gl_check_extension("GL_EXT_texture_compression_dxt1");
	}

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_LOD] =
		ce_gl_check_extension("GL_SGIS_texture_lod") ||
		ce_gl_check_extension("GL_EXT_texture_lod");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_BORDER_CLAMP] =
		ce_gl_check_extension("GL_ARB_texture_border_clamp") ||
		ce_gl_check_extension("GL_SGIS_texture_border_clamp");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_EDGE_CLAMP] =
		ce_gl_check_extension("GL_SGIS_texture_edge_clamp") ||
		ce_gl_check_extension("GL_EXT_texture_edge_clamp");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_FLOAT] =
		ce_gl_check_extension("GL_ARB_texture_float") ||
		ce_gl_check_extension("GL_ATI_texture_float");

	ce_gl_inst.features[CE_GL_FEATURE_ABGR] =
		ce_gl_check_extension("GL_EXT_abgr");

	ce_gl_inst.features[CE_GL_FEATURE_BGRA] =
		ce_gl_check_extension("GL_EXT_bgra");

	ce_gl_inst.features[CE_GL_FEATURE_PACKED_PIXELS] =
		ce_gl_check_extension("GL_EXT_packed_pixels");

	ce_gl_inst.features[CE_GL_FEATURE_GENERATE_MIPMAP] =
		ce_gl_check_extension("GL_SGIS_generate_mipmap");

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

	ce_gl_inst.features[CE_GL_FEATURE_OCCLUSION_QUERY] =
		ce_gl_check_extension("GL_ARB_occlusion_query");

	if (ce_gl_inst.features[CE_GL_FEATURE_OCCLUSION_QUERY]) {
		ce_gl_gen_queries_proc = (CE_GL_GEN_QUERIES_PROC)
			ce_gl_get_proc_address("glGenQueriesARB");
		ce_gl_delete_queries_proc = (CE_GL_DELETE_QUERIES_PROC)
			ce_gl_get_proc_address("glDeleteQueriesARB");
		ce_gl_is_query_proc = (CE_GL_IS_QUERY_PROC)
			ce_gl_get_proc_address("glIsQueryARB");
		ce_gl_begin_query_proc = (CE_GL_BEGIN_QUERY_PROC)
			ce_gl_get_proc_address("glBeginQueryARB");
		ce_gl_end_query_proc = (CE_GL_END_QUERY_PROC)
			ce_gl_get_proc_address("glEndQueryARB");
		ce_gl_get_query_iv_proc = (CE_GL_GET_QUERY_IV_PROC)
			ce_gl_get_proc_address("glGetQueryivARB");
		ce_gl_get_query_object_iv_proc = (CE_GL_GET_QUERY_OBJECT_IV_PROC)
			ce_gl_get_proc_address("glGetQueryObjectivARB");
		ce_gl_get_query_object_uiv_proc = (CE_GL_GET_QUERY_OBJECT_UIV_PROC)
			ce_gl_get_proc_address("glGetQueryObjectuivARB");

		ce_gl_inst.features[CE_GL_FEATURE_OCCLUSION_QUERY] =
			NULL != ce_gl_gen_queries_proc &&
			NULL != ce_gl_delete_queries_proc &&
			NULL != ce_gl_is_query_proc &&
			NULL != ce_gl_begin_query_proc &&
			NULL != ce_gl_end_query_proc &&
			NULL != ce_gl_get_query_iv_proc &&
			NULL != ce_gl_get_query_object_iv_proc &&
			NULL != ce_gl_get_query_object_uiv_proc;

		ce_gl_inst.features[CE_GL_FEATURE_OCCLUSION_QUERY2] =
			ce_gl_inst.features[CE_GL_FEATURE_OCCLUSION_QUERY] &&
			ce_gl_check_extension("GL_ARB_occlusion_query2");
	}

	ce_gl_inst.features[CE_GL_FEATURE_MULTISAMPLE] =
		ce_gl_check_extension("GL_ARB_multisample") ||
		ce_gl_check_extension("GLX_ARB_multisample") ||
		ce_gl_check_extension("WGL_ARB_multisample") ||
		ce_gl_check_extension("GL_EXT_multisample") ||
		ce_gl_check_extension("WGL_EXT_multisample") ||
		ce_gl_check_extension("GL_SGIS_multisample") ||
		ce_gl_check_extension("GLX_SGIS_multisample");

	ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_vertex_buffer_object");

	if (ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT]) {
		ce_gl_bind_buffer_proc = (CE_GL_BIND_BUFFER_PROC)
			ce_gl_get_proc_address("glBindBufferARB");
		ce_gl_delete_buffers_proc = (CE_GL_DELETE_BUFFERS_PROC)
			ce_gl_get_proc_address("glDeleteBuffersARB");
		ce_gl_gen_buffers_proc = (CE_GL_GEN_BUFFERS_PROC)
			ce_gl_get_proc_address("glGenBuffersARB");
		ce_gl_is_buffer_proc = (CE_GL_IS_BUFFER_PROC)
			ce_gl_get_proc_address("glIsBufferARB");
		ce_gl_buffer_data_proc = (CE_GL_BUFFER_DATA_PROC)
			ce_gl_get_proc_address("glBufferDataARB");
		ce_gl_buffer_sub_data_proc = (CE_GL_BUFFER_SUB_DATA_PROC)
			ce_gl_get_proc_address("glBufferSubDataARB");

		ce_gl_get_buffer_sub_data_proc = (CE_GL_GET_BUFFER_SUB_DATA_PROC)
			ce_gl_get_proc_address("glGetBufferSubDataARB");
		ce_gl_map_buffer_proc = (CE_GL_MAP_BUFFER_PROC)
			ce_gl_get_proc_address("glMapBufferARB");
		ce_gl_unmap_buffer_proc = (CE_GL_UNMAP_BUFFER_PROC)
			ce_gl_get_proc_address("glUnmapBufferARB");
		ce_gl_get_buffer_parameter_iv_proc = (CE_GL_GET_BUFFER_PARAMETER_IV_PROC)
			ce_gl_get_proc_address("glGetBufferParameterivARB");
		ce_gl_get_buffer_pointer_v_proc = (CE_GL_GET_BUFFER_POINTER_V_PROC)
			ce_gl_get_proc_address("glGetBufferPointervARB");

		ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT] =
			NULL != ce_gl_bind_buffer_proc &&
			NULL != ce_gl_delete_buffers_proc &&
			NULL != ce_gl_gen_buffers_proc &&
			NULL != ce_gl_is_buffer_proc &&
			NULL != ce_gl_buffer_data_proc &&
			NULL != ce_gl_buffer_sub_data_proc &&
			NULL != ce_gl_get_buffer_sub_data_proc &&
			NULL != ce_gl_map_buffer_proc &&
			NULL != ce_gl_unmap_buffer_proc &&
			NULL != ce_gl_get_buffer_parameter_iv_proc &&
			NULL != ce_gl_get_buffer_pointer_v_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_framebuffer_object") ||
		ce_gl_check_extension("GL_EXT_framebuffer_object");

	if (ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT]) {
		ce_gl_bind_frame_buffer_proc =
			(CE_GL_BIND_FRAME_BUFFER_PROC)
				ce_gl_get_first_proc_address(2, "glBindFramebufferARB",
												"glBindFramebufferEXT");
		ce_gl_delete_frame_buffers_proc =
			(CE_GL_DELETE_FRAME_BUFFERS_PROC)
				ce_gl_get_first_proc_address(2, "glDeleteFramebuffersARB",
												"glDeleteFramebuffersEXT");
		ce_gl_gen_frame_buffers_proc =
			(CE_GL_GEN_FRAME_BUFFERS_PROC)
				ce_gl_get_first_proc_address(2, "glGenFramebuffersARB",
												"glGenFramebuffersEXT");
		ce_gl_check_frame_buffer_status_proc =
			(CE_GL_CHECK_FRAME_BUFFER_STATUS_PROC)
				ce_gl_get_first_proc_address(2, "glCheckFramebufferStatusARB",
												"glCheckFramebufferStatusEXT");
		ce_gl_frame_buffer_texture_2d_proc =
			(CE_GL_FRAME_BUFFER_TEXTURE_2D_PROC)
				ce_gl_get_first_proc_address(2, "glFramebufferTexture2DARB",
												"glFramebufferTexture2DEXT");
		ce_gl_generate_mipmap_proc =
			(CE_GL_GENERATE_MIPMAP_PROC)
				ce_gl_get_first_proc_address(2, "glGenerateMipmapARB",
												"glGenerateMipmapEXT");

		ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT] =
			NULL != ce_gl_bind_frame_buffer_proc &&
			NULL != ce_gl_delete_frame_buffers_proc &&
			NULL != ce_gl_gen_frame_buffers_proc &&
			NULL != ce_gl_check_frame_buffer_status_proc &&
			NULL != ce_gl_frame_buffer_texture_2d_proc &&
			NULL != ce_gl_generate_mipmap_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_PIXEL_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_pixel_buffer_object") ||
		ce_gl_check_extension("GL_EXT_pixel_buffer_object");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_texture_buffer_object") ||
		ce_gl_check_extension("GL_EXT_texture_buffer_object");

	if (ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_BUFFER_OBJECT]) {
		ce_gl_tex_buffer_proc = (CE_GL_TEX_BUFFER_PROC)
			ce_gl_get_proc_address("glTexBufferARB");

		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_BUFFER_OBJECT] =
			NULL != ce_gl_tex_buffer_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_SHADING_LANGUAGE_100] =
		ce_gl_check_extension("GL_ARB_shading_language_100");

	if (ce_gl_inst.features[CE_GL_FEATURE_SHADING_LANGUAGE_100]) {
		ce_gl_inst.features[CE_GL_FEATURE_SHADER_OBJECT] =
			ce_gl_check_extension("GL_ARB_shader_objects");

		if (ce_gl_inst.features[CE_GL_FEATURE_SHADER_OBJECT]) {
			ce_gl_delete_object_proc = (CE_GL_DELETE_OBJECT_PROC)
				ce_gl_get_proc_address("glDeleteObjectARB");
			ce_gl_get_handle_proc = (CE_GL_GET_HANDLE_PROC)
				ce_gl_get_proc_address("glGetHandleARB");
			ce_gl_detach_object_proc = (CE_GL_DETACH_OBJECT_PROC)
				ce_gl_get_proc_address("glDetachObjectARB");
			ce_gl_create_shader_object_proc = (CE_GL_CREATE_SHADER_OBJECT_PROC)
				ce_gl_get_proc_address("glCreateShaderObjectARB");
			ce_gl_shader_source_proc = (CE_GL_SHADER_SOURCE_PROC)
				ce_gl_get_proc_address("glShaderSourceARB");
			ce_gl_compile_shader_proc = (CE_GL_COMPILE_SHADER_PROC)
				ce_gl_get_proc_address("glCompileShaderARB");
			ce_gl_create_program_object_proc = (CE_GL_CREATE_PROGRAM_OBJECT_PROC)
				ce_gl_get_proc_address("glCreateProgramObjectARB");
			ce_gl_attach_object_proc = (CE_GL_ATTACH_OBJECT_PROC)
				ce_gl_get_proc_address("glAttachObjectARB");
			ce_gl_link_program_object_proc = (CE_GL_LINK_PROGRAM_OBJECT_PROC)
				ce_gl_get_proc_address("glLinkProgramARB");
			ce_gl_use_program_object_proc = (CE_GL_USE_PROGRAM_OBJECT_PROC)
				ce_gl_get_proc_address("glUseProgramObjectARB");
			ce_gl_validate_program_object_proc = (CE_GL_VALIDATE_PROGRAM_OBJECT_PROC)
				ce_gl_get_proc_address("glValidateProgramARB");

			ce_gl_inst.features[CE_GL_FEATURE_SHADER_OBJECT] =
				NULL != ce_gl_delete_object_proc &&
				NULL != ce_gl_get_handle_proc &&
				NULL != ce_gl_detach_object_proc &&
				NULL != ce_gl_create_shader_object_proc &&
				NULL != ce_gl_shader_source_proc &&
				NULL != ce_gl_compile_shader_proc &&
				NULL != ce_gl_create_program_object_proc &&
				NULL != ce_gl_attach_object_proc &&
				NULL != ce_gl_link_program_object_proc &&
				NULL != ce_gl_use_program_object_proc &&
				NULL != ce_gl_validate_program_object_proc;
		}
	}

	if (ce_gl_inst.features[CE_GL_FEATURE_SHADER_OBJECT]) {
		ce_gl_inst.features[CE_GL_FEATURE_VERTEX_SHADER] =
			ce_gl_check_extension("GL_ARB_vertex_shader");

		ce_gl_inst.features[CE_GL_FEATURE_FRAGMENT_SHADER] =
			ce_gl_check_extension("GL_ARB_fragment_shader");
	}

	if (ce_gl_inst.features[CE_GL_FEATURE_VERTEX_SHADER]) {
		ce_gl_inst.features[CE_GL_FEATURE_VERTEX_SHADER_TESSELLATOR] =
			ce_gl_check_extension("GL_AMD_vertex_shader_tessellator");

		if (ce_gl_inst.features[CE_GL_FEATURE_VERTEX_SHADER_TESSELLATOR]) {
			ce_gl_vst_set_tessellation_factor_proc =
				(CE_GL_VST_SET_TESSELLATION_FACTOR_PROC)
					ce_gl_get_proc_address("glTessellationFactorAMD");
			ce_gl_vst_set_tessellation_mode_proc =
				(CE_GL_VST_SET_TESSELLATION_MODE_PROC)
					ce_gl_get_proc_address("glTessellationModeAMD");

			ce_gl_inst.features[CE_GL_FEATURE_VERTEX_SHADER_TESSELLATOR] =
				NULL != ce_gl_vst_set_tessellation_factor_proc &&
				NULL != ce_gl_vst_set_tessellation_mode_proc;
		}
	}

	ce_gl_inst.features[CE_GL_FEATURE_PERFORMANCE_MONITOR] =
		ce_gl_check_extension("GL_AMD_performance_monitor");

	if (ce_gl_inst.features[CE_GL_FEATURE_PERFORMANCE_MONITOR]) {
		ce_gl_get_perfmon_groups_proc = (CE_GL_GET_PERFMON_GROUPS_PROC)
			ce_gl_get_proc_address("glGetPerfMonitorGroupsAMD");
		ce_gl_get_perfmon_counters_proc = (CE_GL_GET_PERFMON_COUNTERS_PROC)
			ce_gl_get_proc_address("glGetPerfMonitorCountersAMD");
		ce_gl_get_perfmon_group_str_proc = (CE_GL_GET_PERFMON_GROUP_STR_PROC)
			ce_gl_get_proc_address("glGetPerfMonitorGroupStringAMD");
		ce_gl_get_perfmon_counter_str_proc = (CE_GL_GET_PERFMON_COUNTER_STR_PROC)
			ce_gl_get_proc_address("glGetPerfMonitorCounterStringAMD");
		ce_gl_get_perfmon_counter_info_proc = (CE_GL_GET_PERFMON_COUNTER_INFO_PROC)
			ce_gl_get_proc_address("glGetPerfMonitorCounterInfoAMD");
		ce_gl_gen_perfmons_proc = (CE_GL_GEN_PERFMONS_PROC)
			ce_gl_get_proc_address("glGenPerfMonitorsAMD");
		ce_gl_delete_perfmons_proc = (CE_GL_DELETE_PERFMONS_PROC)
			ce_gl_get_proc_address("glDeletePerfMonitorsAMD");
		ce_gl_select_perfmon_counters_proc = (CE_GL_SELECT_PERFMON_COUNTERS_PROC)
			ce_gl_get_proc_address("glSelectPerfMonitorCountersAMD");
		ce_gl_begin_perfmon_proc = (CE_GL_BEGIN_PERFMON_PROC)
			ce_gl_get_proc_address("glBeginPerfMonitorAMD");
		ce_gl_end_perfmon_proc = (CE_GL_END_PERFMON_PROC)
			ce_gl_get_proc_address("glEndPerfMonitorAMD");
		ce_gl_get_perfmon_counter_data_proc = (CE_GL_GET_PERFMON_COUNTER_DATA_PROC)
			ce_gl_get_proc_address("glGetPerfMonitorCounterDataAMD");
		ce_gl_inst.features[CE_GL_FEATURE_PERFORMANCE_MONITOR] =
			NULL != ce_gl_get_perfmon_groups_proc &&
			NULL != ce_gl_get_perfmon_counters_proc &&
			NULL != ce_gl_get_perfmon_group_str_proc &&
			NULL != ce_gl_get_perfmon_counter_str_proc &&
			NULL != ce_gl_get_perfmon_counter_info_proc &&
			NULL != ce_gl_gen_perfmons_proc &&
			NULL != ce_gl_delete_perfmons_proc &&
			NULL != ce_gl_select_perfmon_counters_proc &&
			NULL != ce_gl_begin_perfmon_proc &&
			NULL != ce_gl_end_perfmon_proc &&
			NULL != ce_gl_get_perfmon_counter_data_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_MEMINFO] =
		ce_gl_check_extension("GL_ATI_meminfo");

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

// texture compression

void ce_gl_compressed_tex_image_2d(GLenum target, GLint level,
									GLenum internal_format,
									GLsizei width, GLsizei height,
									GLint border, GLsizei image_size,
									const GLvoid* data)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_compressed_tex_image_2d_proc);
	(*ce_gl_compressed_tex_image_2d_proc)(target, level,
		internal_format, width, height, border, image_size, data);
}

// window pos

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

// point parameters

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

// occlusion query

void ce_gl_gen_queries(GLsizei n, GLuint* ids)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_gen_queries_proc);
	(*ce_gl_gen_queries_proc)(n, ids);
}

void ce_gl_delete_queries(GLsizei n, const GLuint* ids)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_delete_queries_proc);
	(*ce_gl_delete_queries_proc)(n, ids);
}

GLboolean ce_gl_is_query(GLuint id)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_is_query_proc);
	return (*ce_gl_is_query_proc)(id);
}

void ce_gl_begin_query(GLenum target, GLuint id)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_begin_query_proc);
	(*ce_gl_begin_query_proc)(target, id);
}

void ce_gl_end_query(GLenum target)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_end_query_proc);
	(*ce_gl_end_query_proc)(target);
}

void ce_gl_get_query_iv(GLenum target, GLenum pname, GLint* params)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_query_iv_proc);
	(*ce_gl_get_query_iv_proc)(target, pname, params);
}

void ce_gl_get_query_object_iv(GLuint id, GLenum pname, GLint* params)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_query_object_iv_proc);
	(ce_gl_get_query_object_iv_proc)(id, pname, params);
}

void ce_gl_get_query_object_uiv(GLuint id, GLenum pname, GLuint* params)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_query_object_uiv_proc);
	(*ce_gl_get_query_object_uiv_proc)(id, pname, params);
}

// VBO

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

GLboolean ce_gl_is_buffer(GLuint buffer)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_is_buffer_proc);
	return (*ce_gl_is_buffer_proc)(buffer);
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

void ce_gl_get_buffer_sub_data(GLenum target, GLintptr offset,
								GLsizeiptr size, void* data)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_buffer_sub_data_proc);
	(*ce_gl_get_buffer_sub_data_proc)(target, offset, size, data);
}

void* ce_gl_map_buffer(GLenum target, GLenum access)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_map_buffer_proc);
	return (*ce_gl_map_buffer_proc)(target, access);
}

GLboolean ce_gl_unmap_buffer(GLenum target)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_unmap_buffer_proc);
	return (*ce_gl_unmap_buffer_proc)(target);
}

void ce_gl_get_buffer_parameter_iv(GLenum target, GLenum pname, int* params)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_buffer_parameter_iv_proc);
	(*ce_gl_get_buffer_parameter_iv_proc)(target, pname, params);
}

void ce_gl_get_buffer_pointer_v(GLenum target, GLenum pname, void** params)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_buffer_pointer_v_proc);
	(*ce_gl_get_buffer_pointer_v_proc)(target, pname, params);
}

// FBO

void ce_gl_bind_frame_buffer(GLenum target, GLuint buffer)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_bind_frame_buffer_proc);
	(*ce_gl_bind_frame_buffer_proc)(target, buffer);
}

void ce_gl_delete_frame_buffers(GLsizei n, const GLuint* buffers)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_delete_frame_buffers_proc);
	(*ce_gl_delete_frame_buffers_proc)(n, buffers);
}

void ce_gl_gen_frame_buffers(GLsizei n, GLuint* buffers)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_gen_frame_buffers_proc);
	(*ce_gl_gen_frame_buffers_proc)(n, buffers);
}

GLenum ce_gl_check_frame_buffer_status(GLenum target)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_check_frame_buffer_status_proc);
	return (*ce_gl_check_frame_buffer_status_proc)(target);
}

void ce_gl_frame_buffer_texture_2d(GLenum target, GLenum attachment,
							GLenum tex_target, GLuint texture, GLint level)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_frame_buffer_texture_2d_proc);
	(*ce_gl_frame_buffer_texture_2d_proc)(target, attachment, tex_target,
														texture, level);
}

void ce_gl_generate_mipmap(GLenum target)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_generate_mipmap_proc);
	(*ce_gl_generate_mipmap_proc)(target);
}

// TBO

void ce_gl_tex_buffer(GLenum target, GLenum internal_format, GLuint buffer)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_tex_buffer_proc);
	(*ce_gl_tex_buffer_proc)(target, internal_format, buffer);
}

// shader object

void ce_gl_delete_object(GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_delete_object_proc);
	(*ce_gl_delete_object_proc)(object);
}

GLhandle ce_gl_get_handle(GLenum pname)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_handle_proc);
	return (*ce_gl_get_handle_proc)(pname);
}

void ce_gl_detach_object(GLhandle container, GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_detach_object_proc);
	(*ce_gl_detach_object_proc)(container, object);
}

GLhandle ce_gl_create_shader_object(GLenum type)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_create_shader_object_proc);
	return (*ce_gl_create_shader_object_proc)(type);
}

void ce_gl_shader_source(GLhandle object, GLsizei count,
						const GLchar** string, const int* length)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_shader_source_proc);
	(*ce_gl_shader_source_proc)(object, count, string, length);
}

void ce_gl_compile_shader(GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_compile_shader_proc);
	(*ce_gl_compile_shader_proc)(object);
}

GLhandle ce_gl_create_program_object(void)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_create_program_object_proc);
	return (*ce_gl_create_program_object_proc)();
}

void ce_gl_attach_object(GLhandle container, GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_attach_object_proc);
	(*ce_gl_attach_object_proc)(container, object);
}

void ce_gl_link_program_object(GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_link_program_object_proc);
	(*ce_gl_link_program_object_proc)(object);
}

void ce_gl_use_program_object(GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_use_program_object_proc);
	(*ce_gl_use_program_object_proc)(object);
}

void ce_gl_validate_program_object(GLhandle object)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_validate_program_object_proc);
	(*ce_gl_validate_program_object_proc)(object);
}

// vertex shader tessellator

void ce_gl_vst_set_tessellation_factor(GLfloat factor)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_vst_set_tessellation_factor_proc);
	(*ce_gl_vst_set_tessellation_factor_proc)(factor);
}

void ce_gl_vst_set_tessellation_mode(GLenum mode)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_vst_set_tessellation_mode_proc);
	(*ce_gl_vst_set_tessellation_mode_proc)(mode);
}

// performance monitor

void ce_gl_get_perfmon_groups(GLint* count, GLsizei size, GLuint* groups)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_perfmon_groups_proc);
	(*ce_gl_get_perfmon_groups_proc)(count, size, groups);
}

void ce_gl_get_perfmon_counters(GLuint group, GLint* count, GLint* max_active,
								GLsizei size, GLuint* counters)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_perfmon_counters_proc);
	(*ce_gl_get_perfmon_counters_proc)(group, count, max_active, size, counters);
}

void ce_gl_get_perfmon_group_str(GLuint group, GLsizei size,
								GLsizei* length, char* str)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_perfmon_group_str_proc);
	(*ce_gl_get_perfmon_group_str_proc)(group, size, length, str);
}

void ce_gl_get_perfmon_counter_str(GLuint group, GLuint counter,
									GLsizei size, GLsizei* length, char* str)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_perfmon_counter_str_proc);
	(*ce_gl_get_perfmon_counter_str_proc)(group, counter, size, length, str);
}

void ce_gl_get_perfmon_counter_info(GLuint group, GLuint counter,
									GLenum pname, GLvoid* data)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_perfmon_counter_info_proc);
	(*ce_gl_get_perfmon_counter_info_proc)(group, counter, pname, data);
}

void ce_gl_gen_perfmons(GLsizei n, GLuint* monitors)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_gen_perfmons_proc);
	(*ce_gl_gen_perfmons_proc)(n, monitors);
}

void ce_gl_delete_perfmons(GLsizei n, GLuint* monitors)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_delete_perfmons_proc);
	(*ce_gl_delete_perfmons_proc)(n, monitors);
}

void ce_gl_select_perfmon_counters(GLuint monitor, GLboolean enable,
									GLuint group, GLint count, GLuint* list)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_select_perfmon_counters_proc);
	(*ce_gl_select_perfmon_counters_proc)(monitor, enable, group, count, list);
}

void ce_gl_begin_perfmon(GLuint monitor)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_begin_perfmon_proc);
	(*ce_gl_begin_perfmon_proc)(monitor);
}

void ce_gl_end_perfmon(GLuint monitor)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_end_perfmon_proc);
	(*ce_gl_end_perfmon_proc)(monitor);
}

void ce_gl_get_perfmon_counter_data(GLuint monitor, GLenum pname, GLsizei size,
									GLuint* data, GLint* bytes_written)
{
	assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited");
	assert(NULL != ce_gl_get_perfmon_counter_data_proc);
	(*ce_gl_get_perfmon_counter_data_proc)(monitor, pname, size, data, bytes_written);
}
