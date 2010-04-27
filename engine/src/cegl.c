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

// multitexture
const GLenum CE_GL_TEXTURE0 = 0x84C0;
const GLenum CE_GL_TEXTURE1 = 0x84C1;
const GLenum CE_GL_TEXTURE2 = 0x84C2;
const GLenum CE_GL_TEXTURE3 = 0x84C3;
const GLenum CE_GL_TEXTURE4 = 0x84C4;
const GLenum CE_GL_TEXTURE5 = 0x84C5;
const GLenum CE_GL_TEXTURE6 = 0x84C6;
const GLenum CE_GL_TEXTURE7 = 0x84C7;
const GLenum CE_GL_TEXTURE8 = 0x84C8;
const GLenum CE_GL_TEXTURE9 = 0x84C9;
const GLenum CE_GL_ACTIVE_TEXTURE = 0x84E0;
const GLenum CE_GL_CLIENT_ACTIVE_TEXTURE = 0x84E1;
const GLenum CE_GL_MAX_TEXTURE_UNITS = 0x84E2;

typedef void (APIENTRY *CE_GL_ACTIVE_TEXTURE_PROC)(GLenum);
typedef void (APIENTRY *CE_GL_CLIENT_ACTIVE_TEXTURE_PROC)(GLenum);
typedef void (APIENTRY *CE_GL_MULTI_TEX_COORD_2F_PROC)(GLenum, GLfloat, GLfloat);
typedef void (APIENTRY *CE_GL_MULTI_TEX_COORD_2FV_PROC)(GLenum, const GLfloat*);
typedef void (APIENTRY *CE_GL_MULTI_TEX_COORD_3F_PROC)(GLenum, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *CE_GL_MULTI_TEX_COORD_3FV_PROC)(GLenum, const GLfloat*);

static CE_GL_ACTIVE_TEXTURE_PROC ce_gl_active_texture_proc;
static CE_GL_CLIENT_ACTIVE_TEXTURE_PROC ce_gl_client_active_texture_proc;
static CE_GL_MULTI_TEX_COORD_2F_PROC ce_gl_multi_tex_coord_2f_proc;
static CE_GL_MULTI_TEX_COORD_2FV_PROC ce_gl_multi_tex_coord_2fv_proc;
static CE_GL_MULTI_TEX_COORD_3F_PROC ce_gl_multi_tex_coord_3f_proc;
static CE_GL_MULTI_TEX_COORD_3FV_PROC ce_gl_multi_tex_coord_3fv_proc;

// texture compression
const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT1 = 0x83F1;
const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3 = 0x83F2;

typedef void (APIENTRY *CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC)
	(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*);

static CE_GL_COMPRESSED_TEX_IMAGE_2D_PROC ce_gl_compressed_tex_image_2d_proc;

// texture lod
const GLenum CE_GL_TEXTURE_MAX_LEVEL = 0x813D;

// texture clamp
const GLenum CE_GL_CLAMP_TO_BORDER = 0x812D;
const GLenum CE_GL_CLAMP_TO_EDGE = 0x812F;

// texture float
const GLenum CE_GL_TEXTURE_RED_TYPE = 0x8C10;
const GLenum CE_GL_TEXTURE_GREEN_TYPE = 0x8C11;
const GLenum CE_GL_TEXTURE_BLUE_TYPE = 0x8C12;
const GLenum CE_GL_TEXTURE_ALPHA_TYPE = 0x8C13;
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
const GLenum CE_GL_ARRAY_BUFFER_BINDING = 0x8894;
const GLenum CE_GL_ELEMENT_ARRAY_BUFFER_BINDING = 0x8895;
const GLenum CE_GL_VERTEX_ARRAY_BUFFER_BINDING = 0x8896;
const GLenum CE_GL_NORMAL_ARRAY_BUFFER_BINDING = 0x8897;
const GLenum CE_GL_COLOR_ARRAY_BUFFER_BINDING = 0x8898;
const GLenum CE_GL_INDEX_ARRAY_BUFFER_BINDING = 0x8899;
const GLenum CE_GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING = 0x889A;
const GLenum CE_GL_EDGE_FLAG_ARRAY_BUFFER_BINDING = 0x889B;
const GLenum CE_GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING = 0x889C;
const GLenum CE_GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING = 0x889D;
const GLenum CE_GL_WEIGHT_ARRAY_BUFFER_BINDING = 0x889E;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F;
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
const GLenum CE_GL_BUFFER_SIZE = 0x8764;
const GLenum CE_GL_BUFFER_USAGE = 0x8765;
const GLenum CE_GL_BUFFER_ACCESS = 0x88BB;
const GLenum CE_GL_BUFFER_MAPPED = 0x88BC;
const GLenum CE_GL_BUFFER_MAP_POINTER = 0x88BD;
const GLenum CE_GL_COPY_READ_BUFFER = 0x8F36;
const GLenum CE_GL_COPY_WRITE_BUFFER = 0x8F37;

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
				(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
typedef void* (APIENTRY *CE_GL_MAP_BUFFER_PROC)(GLenum target, GLenum access);
typedef GLboolean (APIENTRY *CE_GL_UNMAP_BUFFER_PROC)(GLenum target);
typedef void (APIENTRY *CE_GL_GET_BUFFER_PARAMETER_IV_PROC)
				(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *CE_GL_GET_BUFFER_POINTER_V_PROC)
				(GLenum target, GLenum pname, GLvoid** params);
typedef void (APIENTRY *CE_GL_COPY_BUFFER_SUB_DATA_PROC)
				(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);

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
static CE_GL_COPY_BUFFER_SUB_DATA_PROC ce_gl_copy_buffer_sub_data_proc;

// FBO
const GLenum CE_GL_FRAME_BUFFER = 0x8D40;
const GLenum CE_GL_READ_FRAME_BUFFER = 0x8CA8;
const GLenum CE_GL_DRAW_FRAME_BUFFER = 0x8CA9;
const GLenum CE_GL_RENDER_BUFFER = 0x8D41;
const GLenum CE_GL_STENCIL_INDEX1 = 0x8D46;
const GLenum CE_GL_STENCIL_INDEX4 = 0x8D47;
const GLenum CE_GL_STENCIL_INDEX8 = 0x8D48;
const GLenum CE_GL_STENCIL_INDEX16 = 0x8D49;
const GLenum CE_GL_RENDER_BUFFER_WIDTH = 0x8D42;
const GLenum CE_GL_RENDER_BUFFER_HEIGHT = 0x8D43;
const GLenum CE_GL_RENDER_BUFFER_INTERNAL_FORMAT = 0x8D44;
const GLenum CE_GL_RENDER_BUFFER_RED_SIZE = 0x8D50;
const GLenum CE_GL_RENDER_BUFFER_GREEN_SIZE = 0x8D51;
const GLenum CE_GL_RENDER_BUFFER_BLUE_SIZE = 0x8D52;
const GLenum CE_GL_RENDER_BUFFER_ALPHA_SIZE = 0x8D53;
const GLenum CE_GL_RENDER_BUFFER_DEPTH_SIZE = 0x8D54;
const GLenum CE_GL_RENDER_BUFFER_STENCIL_SIZE = 0x8D55;
const GLenum CE_GL_RENDER_BUFFER_SAMPLES = 0x8CAB;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_TEXTURE_LAYER = 0x8CD4;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_COLOR_ENCODING = 0x8210;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_COMPONENT_TYPE = 0x8211;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_RED_SIZE = 0x8212;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_GREEN_SIZE = 0x8213;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_BLUE_SIZE = 0x8214;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_ALPHA_SIZE = 0x8215;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_DEPTH_SIZE = 0x8216;
const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_STENCIL_SIZE = 0x8217;
const GLenum CE_GL_COLOR_ATTACHMENT0 = 0x8CE0;
const GLenum CE_GL_COLOR_ATTACHMENT1 = 0x8CE1;
const GLenum CE_GL_COLOR_ATTACHMENT2 = 0x8CE2;
const GLenum CE_GL_COLOR_ATTACHMENT3 = 0x8CE3;
const GLenum CE_GL_COLOR_ATTACHMENT4 = 0x8CE4;
const GLenum CE_GL_COLOR_ATTACHMENT5 = 0x8CE5;
const GLenum CE_GL_COLOR_ATTACHMENT6 = 0x8CE6;
const GLenum CE_GL_COLOR_ATTACHMENT7 = 0x8CE7;
const GLenum CE_GL_COLOR_ATTACHMENT8 = 0x8CE8;
const GLenum CE_GL_COLOR_ATTACHMENT9 = 0x8CE9;
const GLenum CE_GL_DEPTH_ATTACHMENT = 0x8D00;
const GLenum CE_GL_STENCIL_ATTACHMENT = 0x8D20;
const GLenum CE_GL_DEPTH_STENCIL_ATTACHMENT = 0x821A;
const GLenum CE_GL_MAX_SAMPLES = 0x8D57;
const GLenum CE_GL_FRAME_BUFFER_COMPLETE = 0x8CD5;
const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6;
const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7;
const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_DRAW_BUFFER = 0x8CDB;
const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_READ_BUFFER = 0x8CDC;
const GLenum CE_GL_FRAME_BUFFER_UNSUPPORTED = 0x8CDD;
const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_MULTISAMPLE = 0x8D56;
const GLenum CE_GL_FRAME_BUFFER_UNDEFINED = 0x8219;
const GLenum CE_GL_FRAME_BUFFER_BINDING = 0x8CA6;
const GLenum CE_GL_DRAW_FRAME_BUFFER_BINDING = 0x8CA6;
const GLenum CE_GL_READ_FRAME_BUFFER_BINDING = 0x8CAA;
const GLenum CE_GL_RENDER_BUFFER_BINDING = 0x8CA7;
const GLenum CE_GL_MAX_COLOR_ATTACHMENTS = 0x8CDF;
const GLenum CE_GL_MAX_RENDER_BUFFER_SIZE = 0x84E8;
const GLenum CE_GL_INVALID_FRAME_BUFFER_OPERATION = 0x0506;
const GLenum CE_GL_DEPTH_STENCIL = 0x84F9;
const GLenum CE_GL_UNSIGNED_INT_24_8 = 0x84FA;
const GLenum CE_GL_DEPTH24_STENCIL8 = 0x88F0;
const GLenum CE_GL_TEXTURE_STENCIL_SIZE = 0x88F1;

typedef GLboolean (APIENTRY *CE_GL_IS_RENDER_BUFFER_PROC)(GLuint);
typedef void (APIENTRY *CE_GL_BIND_RENDER_BUFFER_PROC)(GLenum, GLuint);
typedef void (APIENTRY *CE_GL_DELETE_RENDER_BUFFERS_PROC)(GLsizei, const GLuint*);
typedef void (APIENTRY *CE_GL_GEN_RENDER_BUFFERS_PROC)(GLsizei, GLuint*);
typedef void (APIENTRY *CE_GL_RENDER_BUFFER_STORAGE_PROC)
			(GLenum, GLenum, GLsizei, GLsizei);
typedef void (APIENTRY *CE_GL_RENDER_BUFFER_STORAGE_MULTISAMPLE_PROC)
			(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
typedef void (APIENTRY *CE_GL_GET_RENDER_BUFFER_PARAMETER_IV_PROC)(GLenum, GLenum, GLint*);
typedef GLboolean (APIENTRY *CE_GL_IS_FRAME_BUFFER_PROC)(GLuint);
typedef void (APIENTRY *CE_GL_BIND_FRAME_BUFFER_PROC)(GLenum, GLuint);
typedef void (APIENTRY *CE_GL_DELETE_FRAME_BUFFERS_PROC)(GLsizei, const GLuint*);
typedef void (APIENTRY *CE_GL_GEN_FRAME_BUFFERS_PROC)(GLsizei, GLuint*);
typedef GLenum (APIENTRY *CE_GL_CHECK_FRAME_BUFFER_STATUS_PROC)(GLenum);
typedef void (APIENTRY *CE_GL_FRAME_BUFFER_TEXTURE_1D_PROC)
			(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (APIENTRY *CE_GL_FRAME_BUFFER_TEXTURE_2D_PROC)
			(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (APIENTRY *CE_GL_FRAME_BUFFER_TEXTURE_3D_PROC)
			(GLenum, GLenum, GLenum, GLuint, GLint, GLint);
typedef void (APIENTRY *CE_GL_FRAME_BUFFER_TEXTURE_LAYER_PROC)
			(GLenum, GLenum, GLuint, GLint, GLint);
typedef void (APIENTRY *CE_GL_FRAME_BUFFER_RENDER_BUFFER_PROC)
			(GLenum, GLenum, GLenum, GLuint);
typedef void (APIENTRY *CE_GL_GET_FRAME_BUFFER_ATTACHMENT_PARAMETER_IV_PROC)
			(GLenum, GLenum, GLenum, GLint*);
typedef void (APIENTRY *CE_GL_BLIT_FRAME_BUFFER_PROC)(GLint, GLint,
			GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);

typedef void (APIENTRY *CE_GL_GENERATE_MIPMAP_PROC)(GLenum);

static CE_GL_IS_RENDER_BUFFER_PROC ce_gl_is_render_buffer_proc;
static CE_GL_BIND_RENDER_BUFFER_PROC ce_gl_bind_render_buffer_proc;
static CE_GL_DELETE_RENDER_BUFFERS_PROC ce_gl_delete_render_buffers_proc;
static CE_GL_GEN_RENDER_BUFFERS_PROC ce_gl_gen_render_buffers_proc;
static CE_GL_RENDER_BUFFER_STORAGE_PROC ce_gl_render_buffer_storage_proc;
static CE_GL_RENDER_BUFFER_STORAGE_MULTISAMPLE_PROC ce_gl_render_buffer_storage_multisample_proc;
static CE_GL_GET_RENDER_BUFFER_PARAMETER_IV_PROC ce_gl_get_render_buffer_parameter_iv_proc;
static CE_GL_IS_FRAME_BUFFER_PROC ce_gl_is_frame_buffer_proc;
static CE_GL_BIND_FRAME_BUFFER_PROC ce_gl_bind_frame_buffer_proc;
static CE_GL_DELETE_FRAME_BUFFERS_PROC ce_gl_delete_frame_buffers_proc;
static CE_GL_GEN_FRAME_BUFFERS_PROC ce_gl_gen_frame_buffers_proc;
static CE_GL_CHECK_FRAME_BUFFER_STATUS_PROC ce_gl_check_frame_buffer_status_proc;
static CE_GL_FRAME_BUFFER_TEXTURE_1D_PROC ce_gl_frame_buffer_texture_1d_proc;
static CE_GL_FRAME_BUFFER_TEXTURE_2D_PROC ce_gl_frame_buffer_texture_2d_proc;
static CE_GL_FRAME_BUFFER_TEXTURE_3D_PROC ce_gl_frame_buffer_texture_3d_proc;
static CE_GL_FRAME_BUFFER_TEXTURE_LAYER_PROC ce_gl_frame_buffer_texture_layer_proc;
static CE_GL_FRAME_BUFFER_RENDER_BUFFER_PROC ce_gl_frame_buffer_render_buffer_proc;
static CE_GL_GET_FRAME_BUFFER_ATTACHMENT_PARAMETER_IV_PROC ce_gl_get_frame_buffer_attachment_parameter_iv_proc;
static CE_GL_BLIT_FRAME_BUFFER_PROC ce_gl_blit_frame_buffer_proc;
static CE_GL_GENERATE_MIPMAP_PROC ce_gl_generate_mipmap_proc;

// PBO
const GLenum CE_GL_PIXEL_PACK_BUFFER = 0x88EB;
const GLenum CE_GL_PIXEL_UNPACK_BUFFER = 0x88EC;
const GLenum CE_GL_PIXEL_PACK_BUFFER_BINDING = 0x88ED;
const GLenum CE_GL_PIXEL_UNPACK_BUFFER_BINDING = 0x88EF;

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

typedef void (APIENTRY *CE_GL_DELETE_OBJECT_PROC)(GLhandle);
typedef GLhandle (APIENTRY *CE_GL_GET_HANDLE_PROC)(GLenum);
typedef void (APIENTRY *CE_GL_DETACH_OBJECT_PROC)(GLhandle, GLhandle);
typedef GLhandle (APIENTRY *CE_GL_CREATE_SHADER_OBJECT_PROC)(GLenum);
typedef void (APIENTRY *CE_GL_SHADER_SOURCE_PROC)
			(GLhandle, GLsizei, const GLchar**, const GLint*);
typedef void (APIENTRY *CE_GL_COMPILE_SHADER_PROC)(GLhandle);
typedef GLhandle (APIENTRY *CE_GL_CREATE_PROGRAM_OBJECT_PROC)(void);
typedef void (APIENTRY *CE_GL_ATTACH_OBJECT_PROC)(GLhandle, GLhandle);
typedef void (APIENTRY *CE_GL_LINK_PROGRAM_OBJECT_PROC)(GLhandle);
typedef void (APIENTRY *CE_GL_USE_PROGRAM_OBJECT_PROC)(GLhandle);
typedef void (APIENTRY *CE_GL_VALIDATE_PROGRAM_OBJECT_PROC)(GLhandle);
typedef void (APIENTRY *CE_GL_UNIFORM_1F_PROC)(GLint, GLfloat);
typedef void (APIENTRY *CE_GL_UNIFORM_2F_PROC)(GLint, GLfloat, GLfloat);
typedef void (APIENTRY *CE_GL_UNIFORM_3F_PROC)(GLint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *CE_GL_UNIFORM_4F_PROC)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *CE_GL_UNIFORM_1I_PROC)(GLint, GLint);
typedef void (APIENTRY *CE_GL_UNIFORM_2I_PROC)(GLint, GLint, GLint);
typedef void (APIENTRY *CE_GL_UNIFORM_3I_PROC)(GLint, GLint, GLint, GLint);
typedef void (APIENTRY *CE_GL_UNIFORM_4I_PROC)(GLint, GLint, GLint, GLint, GLint);
typedef void (APIENTRY *CE_GL_UNIFORM_1FV_PROC)(GLint, GLsizei, const GLfloat*);
typedef void (APIENTRY *CE_GL_UNIFORM_2FV_PROC)(GLint, GLsizei, const GLfloat*);
typedef void (APIENTRY *CE_GL_UNIFORM_3FV_PROC)(GLint, GLsizei, const GLfloat*);
typedef void (APIENTRY *CE_GL_UNIFORM_4FV_PROC)(GLint, GLsizei, const GLfloat*);
typedef void (APIENTRY *CE_GL_UNIFORM_1IV_PROC)(GLint, GLsizei, const GLint*);
typedef void (APIENTRY *CE_GL_UNIFORM_2IV_PROC)(GLint, GLsizei, const GLint*);
typedef void (APIENTRY *CE_GL_UNIFORM_3IV_PROC)(GLint, GLsizei, const GLint*);
typedef void (APIENTRY *CE_GL_UNIFORM_4IV_PROC)(GLint, GLsizei, const GLint*);
typedef void (APIENTRY *CE_GL_UNIFORM_MATRIX_2FV_PROC)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (APIENTRY *CE_GL_UNIFORM_MATRIX_3FV_PROC)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (APIENTRY *CE_GL_UNIFORM_MATRIX_4FV_PROC)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (APIENTRY *CE_GL_GET_OBJECT_PARAMETER_FV_PROC)(GLhandle, GLenum, GLfloat*);
typedef void (APIENTRY *CE_GL_GET_OBJECT_PARAMETER_IV_PROC)(GLhandle, GLenum, GLint*);
typedef void (APIENTRY *CE_GL_GET_INFO_LOG_PROC)(GLhandle, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRY *CE_GL_GET_ATTACHED_OBJECTS_PROC)(GLhandle, GLsizei, GLsizei*, GLhandle*);
typedef GLint (APIENTRY *CE_GL_GET_UNIFORM_LOCATION_PROC)(GLhandle, const GLchar*);
typedef void (APIENTRY *CE_GL_GET_ACTIVE_UNIFORM_PROC)(GLhandle, GLuint, GLsizei,
									GLsizei*, GLint*, GLenum*, GLchar*);
typedef void (APIENTRY *CE_GL_GET_UNIFORM_FV_PROC)(GLhandle, GLint, GLfloat*);
typedef void (APIENTRY *CE_GL_GET_UNIFORM_IV_PROC)(GLhandle, GLint, GLint*);
typedef void (APIENTRY *CE_GL_GET_SHADER_SOURCE_PROC)(GLhandle, GLsizei, GLsizei*, GLchar*);

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
static CE_GL_UNIFORM_1F_PROC ce_gl_uniform_1f_proc;
static CE_GL_UNIFORM_2F_PROC ce_gl_uniform_2f_proc;
static CE_GL_UNIFORM_3F_PROC ce_gl_uniform_3f_proc;
static CE_GL_UNIFORM_4F_PROC ce_gl_uniform_4f_proc;
static CE_GL_UNIFORM_1I_PROC ce_gl_uniform_1i_proc;
static CE_GL_UNIFORM_2I_PROC ce_gl_uniform_2i_proc;
static CE_GL_UNIFORM_3I_PROC ce_gl_uniform_3i_proc;
static CE_GL_UNIFORM_4I_PROC ce_gl_uniform_4i_proc;
static CE_GL_UNIFORM_1FV_PROC ce_gl_uniform_1fv_proc;
static CE_GL_UNIFORM_2FV_PROC ce_gl_uniform_2fv_proc;
static CE_GL_UNIFORM_3FV_PROC ce_gl_uniform_3fv_proc;
static CE_GL_UNIFORM_4FV_PROC ce_gl_uniform_4fv_proc;
static CE_GL_UNIFORM_1IV_PROC ce_gl_uniform_1iv_proc;
static CE_GL_UNIFORM_2IV_PROC ce_gl_uniform_2iv_proc;
static CE_GL_UNIFORM_3IV_PROC ce_gl_uniform_3iv_proc;
static CE_GL_UNIFORM_4IV_PROC ce_gl_uniform_4iv_proc;
static CE_GL_UNIFORM_MATRIX_2FV_PROC ce_gl_uniform_matrix_2fv_proc;
static CE_GL_UNIFORM_MATRIX_3FV_PROC ce_gl_uniform_matrix_3fv_proc;
static CE_GL_UNIFORM_MATRIX_4FV_PROC ce_gl_uniform_matrix_4fv_proc;
static CE_GL_GET_OBJECT_PARAMETER_FV_PROC ce_gl_get_object_parameter_fv_proc;
static CE_GL_GET_OBJECT_PARAMETER_IV_PROC ce_gl_get_object_parameter_iv_proc;
static CE_GL_GET_INFO_LOG_PROC ce_gl_get_info_log_proc;
static CE_GL_GET_ATTACHED_OBJECTS_PROC ce_gl_get_attached_objects_proc;
static CE_GL_GET_UNIFORM_LOCATION_PROC ce_gl_get_uniform_location_proc;
static CE_GL_GET_ACTIVE_UNIFORM_PROC ce_gl_get_active_uniform_proc;
static CE_GL_GET_UNIFORM_FV_PROC ce_gl_get_uniform_fv_proc;
static CE_GL_GET_UNIFORM_IV_PROC ce_gl_get_uniform_iv_proc;
static CE_GL_GET_SHADER_SOURCE_PROC ce_gl_get_shader_source_proc;

// vertex shader
const GLenum CE_GL_VERTEX_SHADER = 0x8B31;
const GLenum CE_GL_MAX_VERTEX_UNIFORM_COMPONENTS = 0x8B4A;
const GLenum CE_GL_MAX_VARYING_FLOATS = 0x8B4B;
const GLenum CE_GL_MAX_VERTEX_ATTRIBS = 0x8869;
const GLenum CE_GL_MAX_TEXTURE_IMAGE_UNITS = 0x8872;
const GLenum CE_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C;
const GLenum CE_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
const GLenum CE_GL_MAX_TEXTURE_COORDS = 0x8871;
const GLenum CE_GL_VERTEX_PROGRAM_POINT_SIZE = 0x8642;
const GLenum CE_GL_VERTEX_PROGRAM_TWO_SIDE = 0x8643;
const GLenum CE_GL_OBJECT_ACTIVE_ATTRIBUTES = 0x8B89;
const GLenum CE_GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH = 0x8B8A;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_ENABLED = 0x8622;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_SIZE = 0x8623;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_STRIDE = 0x8624;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_TYPE = 0x8625;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A;
const GLenum CE_GL_CURRENT_VERTEX_ATTRIB = 0x8626;
const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_POINTER = 0x8645;

// fragment shader
const GLenum CE_GL_FRAGMENT_SHADER = 0x8B30;
const GLenum CE_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = 0x8B49;
const GLenum CE_GL_FRAGMENT_SHADER_DERIVATIVE_HINT = 0x8B8B;

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
						(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRY *CE_GL_GET_PERFMON_COUNTER_STR_PROC)
						(GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
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
typedef void (*ce_gl_ext_proc_ptr)();

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
		"multitexture",
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
		"copy buffer",
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

	ce_gl_inst.features[CE_GL_FEATURE_MULTITEXTURE] =
		ce_gl_check_extension("GL_ARB_multitexture");

	if (ce_gl_inst.features[CE_GL_FEATURE_MULTITEXTURE]) {
		ce_gl_active_texture_proc = (CE_GL_ACTIVE_TEXTURE_PROC)
			ce_gl_get_proc_address("glActiveTextureARB");
		ce_gl_client_active_texture_proc = (CE_GL_CLIENT_ACTIVE_TEXTURE_PROC)
			ce_gl_get_proc_address("glClientActiveTextureARB");
		ce_gl_multi_tex_coord_2f_proc = (CE_GL_MULTI_TEX_COORD_2F_PROC)
			ce_gl_get_proc_address("glMultiTexCoord2fARB");
		ce_gl_multi_tex_coord_2fv_proc = (CE_GL_MULTI_TEX_COORD_2FV_PROC)
			ce_gl_get_proc_address("glMultiTexCoord2fvARB");
		ce_gl_multi_tex_coord_3f_proc = (CE_GL_MULTI_TEX_COORD_3F_PROC)
			ce_gl_get_proc_address("glMultiTexCoord3fARB");
		ce_gl_multi_tex_coord_3fv_proc = (CE_GL_MULTI_TEX_COORD_3FV_PROC)
			ce_gl_get_proc_address("glMultiTexCoord3fvARB");

		ce_gl_inst.features[CE_GL_FEATURE_MULTITEXTURE] =
			NULL != ce_gl_active_texture_proc &&
			NULL != ce_gl_client_active_texture_proc &&
			NULL != ce_gl_multi_tex_coord_2f_proc &&
			NULL != ce_gl_multi_tex_coord_2fv_proc &&
			NULL != ce_gl_multi_tex_coord_3f_proc &&
			NULL != ce_gl_multi_tex_coord_3fv_proc;
	}

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
	}

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
		ce_gl_check_extension("GL_EXT_texture_compression_s3tc");

	ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1] =
		ce_gl_inst.features[CE_GL_FEATURE_TEXTURE_COMPRESSION] &&
		ce_gl_check_extension("GL_EXT_texture_compression_dxt1");

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

	ce_gl_inst.features[CE_GL_FEATURE_COPY_BUFFER] =
		ce_gl_inst.features[CE_GL_FEATURE_VERTEX_BUFFER_OBJECT] &&
		(ce_gl_check_extension("GL_ARB_copy_buffer") ||
		ce_gl_check_extension("GL_EXT_copy_buffer"));

	if (ce_gl_inst.features[CE_GL_FEATURE_COPY_BUFFER]) {
		ce_gl_copy_buffer_sub_data_proc = (CE_GL_COPY_BUFFER_SUB_DATA_PROC)
			ce_gl_get_first_proc_address(2, "glCopyBufferSubDataARB",
											"glCopyBufferSubDataEXT");

		ce_gl_inst.features[CE_GL_FEATURE_COPY_BUFFER] =
			NULL != ce_gl_copy_buffer_sub_data_proc;
	}

	ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT] =
		ce_gl_check_extension("GL_ARB_framebuffer_object");

	if (!ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT]) {
		ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT] =
			ce_gl_check_extension("GL_EXT_framebuffer_object") &&
			ce_gl_check_extension("GL_EXT_framebuffer_blit") &&
			ce_gl_check_extension("GL_EXT_framebuffer_multisample") &&
			ce_gl_check_extension("GL_EXT_packed_depth_stencil");
	}

	if (ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT]) {
		ce_gl_is_render_buffer_proc = (CE_GL_IS_RENDER_BUFFER_PROC)
			ce_gl_get_first_proc_address(2, "glIsRenderbufferARB",
											"glIsRenderbufferEXT");
		ce_gl_bind_render_buffer_proc = (CE_GL_BIND_RENDER_BUFFER_PROC)
			ce_gl_get_first_proc_address(2, "glBindRenderbufferARB",
											"glBindRenderbufferEXT");
		ce_gl_delete_render_buffers_proc = (CE_GL_DELETE_RENDER_BUFFERS_PROC)
			ce_gl_get_first_proc_address(2, "glDeleteRenderbuffersARB",
											"glDeleteRenderbuffersEXT");
		ce_gl_gen_render_buffers_proc = (CE_GL_GEN_RENDER_BUFFERS_PROC)
			ce_gl_get_first_proc_address(2, "glGenRenderbuffersARB",
											"glGenRenderbuffersEXT");
		ce_gl_render_buffer_storage_proc = (CE_GL_RENDER_BUFFER_STORAGE_PROC)
			ce_gl_get_first_proc_address(2, "glRenderbufferStorageARB",
											"glRenderbufferStorageEXT");
		ce_gl_render_buffer_storage_multisample_proc =
			(CE_GL_RENDER_BUFFER_STORAGE_MULTISAMPLE_PROC)
				ce_gl_get_first_proc_address(2, "glRenderbufferStorageMultisampleARB",
												"glRenderbufferStorageMultisampleEXT");
		ce_gl_get_render_buffer_parameter_iv_proc =
			(CE_GL_GET_RENDER_BUFFER_PARAMETER_IV_PROC)
				ce_gl_get_first_proc_address(2, "glGetRenderbufferParameterivARB",
												"glGetRenderbufferParameterivEXT");
		ce_gl_is_frame_buffer_proc = (CE_GL_IS_FRAME_BUFFER_PROC)
			ce_gl_get_first_proc_address(2, "glIsFramebufferARB",
											"glIsFramebufferEXT");

		ce_gl_bind_frame_buffer_proc = (CE_GL_BIND_FRAME_BUFFER_PROC)
			ce_gl_get_first_proc_address(2, "glBindFramebufferARB",
											"glBindFramebufferEXT");
		ce_gl_delete_frame_buffers_proc = (CE_GL_DELETE_FRAME_BUFFERS_PROC)
			ce_gl_get_first_proc_address(2, "glDeleteFramebuffersARB",
											"glDeleteFramebuffersEXT");
		ce_gl_gen_frame_buffers_proc = (CE_GL_GEN_FRAME_BUFFERS_PROC)
			ce_gl_get_first_proc_address(2, "glGenFramebuffersARB",
											"glGenFramebuffersEXT");
		ce_gl_check_frame_buffer_status_proc =
			(CE_GL_CHECK_FRAME_BUFFER_STATUS_PROC)
				ce_gl_get_first_proc_address(2, "glCheckFramebufferStatusARB",
												"glCheckFramebufferStatusEXT");
		ce_gl_frame_buffer_texture_1d_proc = (CE_GL_FRAME_BUFFER_TEXTURE_1D_PROC)
			ce_gl_get_first_proc_address(2, "glFramebufferTexture1DARB",
											"glFramebufferTexture1DEXT");
		ce_gl_frame_buffer_texture_2d_proc = (CE_GL_FRAME_BUFFER_TEXTURE_2D_PROC)
			ce_gl_get_first_proc_address(2, "glFramebufferTexture2DARB",
											"glFramebufferTexture2DEXT");
		ce_gl_frame_buffer_texture_3d_proc =
			(CE_GL_FRAME_BUFFER_TEXTURE_3D_PROC)
				ce_gl_get_first_proc_address(2, "glFramebufferTexture3DARB",
												"glFramebufferTexture3DEXT");
		ce_gl_frame_buffer_texture_layer_proc =
			(CE_GL_FRAME_BUFFER_TEXTURE_LAYER_PROC)
				ce_gl_get_first_proc_address(2, "glFramebufferTextureLayerARB",
												"glFramebufferTextureLayerEXT");
		ce_gl_frame_buffer_render_buffer_proc =
			(CE_GL_FRAME_BUFFER_RENDER_BUFFER_PROC)
				ce_gl_get_first_proc_address(2, "glFramebufferRenderbufferARB",
												"glFramebufferRenderbufferEXT");
		ce_gl_get_frame_buffer_attachment_parameter_iv_proc =
			(CE_GL_GET_FRAME_BUFFER_ATTACHMENT_PARAMETER_IV_PROC)
				ce_gl_get_first_proc_address(2, "glGetFramebufferAttachmentParameterivARB",
												"glGetFramebufferAttachmentParameterivEXT");
		ce_gl_blit_frame_buffer_proc = (CE_GL_BLIT_FRAME_BUFFER_PROC)
			ce_gl_get_first_proc_address(2, "glBlitFramebufferARB",
											"glBlitFramebufferEXT");
		ce_gl_generate_mipmap_proc = (CE_GL_GENERATE_MIPMAP_PROC)
			ce_gl_get_first_proc_address(2, "glGenerateMipmapARB",
											"glGenerateMipmapEXT");

		ce_gl_inst.features[CE_GL_FEATURE_FRAME_BUFFER_OBJECT] =
			NULL != ce_gl_is_render_buffer_proc &&
			NULL != ce_gl_bind_render_buffer_proc &&
			NULL != ce_gl_delete_render_buffers_proc &&
			NULL != ce_gl_gen_render_buffers_proc &&
			NULL != ce_gl_render_buffer_storage_proc &&
			NULL != ce_gl_render_buffer_storage_multisample_proc &&
			NULL != ce_gl_get_render_buffer_parameter_iv_proc &&
			NULL != ce_gl_is_frame_buffer_proc &&
			NULL != ce_gl_bind_frame_buffer_proc &&
			NULL != ce_gl_delete_frame_buffers_proc &&
			NULL != ce_gl_gen_frame_buffers_proc &&
			NULL != ce_gl_check_frame_buffer_status_proc &&
			NULL != ce_gl_frame_buffer_texture_1d_proc &&
			NULL != ce_gl_frame_buffer_texture_2d_proc &&
			NULL != ce_gl_frame_buffer_texture_3d_proc &&
			NULL != ce_gl_frame_buffer_texture_layer_proc &&
			NULL != ce_gl_frame_buffer_render_buffer_proc &&
			NULL != ce_gl_get_frame_buffer_attachment_parameter_iv_proc &&
			NULL != ce_gl_blit_frame_buffer_proc &&
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
			ce_gl_validate_program_object_proc =
				(CE_GL_VALIDATE_PROGRAM_OBJECT_PROC)
					ce_gl_get_proc_address("glValidateProgramARB");
			ce_gl_uniform_1f_proc = (CE_GL_UNIFORM_1F_PROC)
				ce_gl_get_proc_address("glUniform1fARB");
			ce_gl_uniform_2f_proc = (CE_GL_UNIFORM_2F_PROC)
				ce_gl_get_proc_address("glUniform2fARB");
			ce_gl_uniform_3f_proc = (CE_GL_UNIFORM_3F_PROC)
				ce_gl_get_proc_address("glUniform3fARB");
			ce_gl_uniform_4f_proc = (CE_GL_UNIFORM_4F_PROC)
				ce_gl_get_proc_address("glUniform4fARB");
			ce_gl_uniform_1i_proc = (CE_GL_UNIFORM_1I_PROC)
				ce_gl_get_proc_address("glUniform1iARB");
			ce_gl_uniform_2i_proc = (CE_GL_UNIFORM_2I_PROC)
				ce_gl_get_proc_address("glUniform2iARB");
			ce_gl_uniform_3i_proc = (CE_GL_UNIFORM_3I_PROC)
				ce_gl_get_proc_address("glUniform3iARB");
			ce_gl_uniform_4i_proc = (CE_GL_UNIFORM_4I_PROC)
				ce_gl_get_proc_address("glUniform4iARB");
			ce_gl_uniform_1fv_proc = (CE_GL_UNIFORM_1FV_PROC)
				ce_gl_get_proc_address("glUniform1fvARB");
			ce_gl_uniform_2fv_proc = (CE_GL_UNIFORM_2FV_PROC)
				ce_gl_get_proc_address("glUniform2fvARB");
			ce_gl_uniform_3fv_proc = (CE_GL_UNIFORM_3FV_PROC)
				ce_gl_get_proc_address("glUniform3fvARB");
			ce_gl_uniform_4fv_proc = (CE_GL_UNIFORM_4FV_PROC)
				ce_gl_get_proc_address("glUniform4fvARB");
			ce_gl_uniform_1iv_proc = (CE_GL_UNIFORM_1IV_PROC)
				ce_gl_get_proc_address("glUniform1ivARB");
			ce_gl_uniform_2iv_proc = (CE_GL_UNIFORM_2IV_PROC)
				ce_gl_get_proc_address("glUniform2ivARB");
			ce_gl_uniform_3iv_proc = (CE_GL_UNIFORM_3IV_PROC)
				ce_gl_get_proc_address("glUniform3ivARB");
			ce_gl_uniform_4iv_proc = (CE_GL_UNIFORM_4IV_PROC)
				ce_gl_get_proc_address("glUniform4ivARB");
			ce_gl_uniform_matrix_2fv_proc = (CE_GL_UNIFORM_MATRIX_2FV_PROC)
				ce_gl_get_proc_address("glUniformMatrix2fvARB");
			ce_gl_uniform_matrix_3fv_proc = (CE_GL_UNIFORM_MATRIX_3FV_PROC)
				ce_gl_get_proc_address("glUniformMatrix3fvARB");
			ce_gl_uniform_matrix_4fv_proc = (CE_GL_UNIFORM_MATRIX_4FV_PROC)
				ce_gl_get_proc_address("glUniformMatrix4fvARB");
			ce_gl_get_object_parameter_fv_proc =
				(CE_GL_GET_OBJECT_PARAMETER_FV_PROC)
					ce_gl_get_proc_address("glGetObjectParameterfvARB");
			ce_gl_get_object_parameter_iv_proc =
				(CE_GL_GET_OBJECT_PARAMETER_IV_PROC)
					ce_gl_get_proc_address("glGetObjectParameterivARB");
			ce_gl_get_info_log_proc = (CE_GL_GET_INFO_LOG_PROC)
				ce_gl_get_proc_address("glGetInfoLogARB");
			ce_gl_get_attached_objects_proc = (CE_GL_GET_ATTACHED_OBJECTS_PROC)
				ce_gl_get_proc_address("glGetAttachedObjectsARB");
			ce_gl_get_uniform_location_proc = (CE_GL_GET_UNIFORM_LOCATION_PROC)
				ce_gl_get_proc_address("glGetUniformLocationARB");
			ce_gl_get_active_uniform_proc = (CE_GL_GET_ACTIVE_UNIFORM_PROC)
				ce_gl_get_proc_address("glGetActiveUniformARB");
			ce_gl_get_uniform_fv_proc = (CE_GL_GET_UNIFORM_FV_PROC)
				ce_gl_get_proc_address("glGetUniformfvARB");
			ce_gl_get_uniform_iv_proc = (CE_GL_GET_UNIFORM_IV_PROC)
				ce_gl_get_proc_address("glGetUniformivARB");
			ce_gl_get_shader_source_proc = (CE_GL_GET_SHADER_SOURCE_PROC)
				ce_gl_get_proc_address("glGetShaderSourceARB");

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
				NULL != ce_gl_validate_program_object_proc &&
				NULL != ce_gl_uniform_1f_proc &&
				NULL != ce_gl_uniform_2f_proc &&
				NULL != ce_gl_uniform_3f_proc &&
				NULL != ce_gl_uniform_4f_proc &&
				NULL != ce_gl_uniform_1i_proc &&
				NULL != ce_gl_uniform_2i_proc &&
				NULL != ce_gl_uniform_3i_proc &&
				NULL != ce_gl_uniform_4i_proc &&
				NULL != ce_gl_uniform_1fv_proc &&
				NULL != ce_gl_uniform_2fv_proc &&
				NULL != ce_gl_uniform_3fv_proc &&
				NULL != ce_gl_uniform_4fv_proc &&
				NULL != ce_gl_uniform_1iv_proc &&
				NULL != ce_gl_uniform_2iv_proc &&
				NULL != ce_gl_uniform_3iv_proc &&
				NULL != ce_gl_uniform_4iv_proc &&
				NULL != ce_gl_uniform_matrix_2fv_proc &&
				NULL != ce_gl_uniform_matrix_3fv_proc &&
				NULL != ce_gl_uniform_matrix_4fv_proc &&
				NULL != ce_gl_get_object_parameter_fv_proc &&
				NULL != ce_gl_get_object_parameter_iv_proc &&
				NULL != ce_gl_get_info_log_proc &&
				NULL != ce_gl_get_attached_objects_proc &&
				NULL != ce_gl_get_uniform_location_proc &&
				NULL != ce_gl_get_active_uniform_proc &&
				NULL != ce_gl_get_uniform_fv_proc &&
				NULL != ce_gl_get_uniform_iv_proc &&
				NULL != ce_gl_get_shader_source_proc;
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

#define CE_GL_CHECK_PROC_POINTER(P) \
assert(ce_gl_inst.inited && "The gl subsystem has not yet been inited"); \
assert(NULL != P);

// multitexture

void ce_gl_active_texture(GLenum target)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_active_texture_proc);
	(*ce_gl_active_texture_proc)(target);
}

void ce_gl_client_active_texture(GLenum target)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_client_active_texture_proc);
	(*ce_gl_client_active_texture_proc)(target);
}

void ce_gl_multi_tex_coord_2f(GLenum target, GLfloat s, GLfloat t)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_multi_tex_coord_2f_proc);
	(*ce_gl_multi_tex_coord_2f_proc)(target, s, t);
}

void ce_gl_multi_tex_coord_2fv(GLenum target, const GLfloat* v)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_multi_tex_coord_2fv_proc);
	(*ce_gl_multi_tex_coord_2fv_proc)(target, v);
}

void ce_gl_multi_tex_coord_3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_multi_tex_coord_3f_proc);
	(*ce_gl_multi_tex_coord_3f_proc)(target, s, t, r);
}

void ce_gl_multi_tex_coord_3fv(GLenum target, const GLfloat* v)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_multi_tex_coord_3fv_proc);
	(*ce_gl_multi_tex_coord_3fv_proc)(target, v);
}

// texture compression

void ce_gl_compressed_tex_image_2d(GLenum target, GLint level,
									GLenum internal_format,
									GLsizei width, GLsizei height,
									GLint border, GLsizei image_size,
									const GLvoid* data)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_compressed_tex_image_2d_proc);
	(*ce_gl_compressed_tex_image_2d_proc)(target, level,
		internal_format, width, height, border, image_size, data);
}

// window pos

void ce_gl_window_pos_2f(GLfloat x, GLfloat y)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_window_pos_2f_proc);
	(*ce_gl_window_pos_2f_proc)(x, y);
}

void ce_gl_window_pos_2fv(const GLfloat* v)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_window_pos_2fv_proc);
	(*ce_gl_window_pos_2fv_proc)(v);
}

void ce_gl_window_pos_2i(GLint x, GLint y)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_window_pos_2i_proc);
	(*ce_gl_window_pos_2i_proc)(x, y);
}

void ce_gl_window_pos_2iv(const GLint* v)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_window_pos_2iv_proc);
	(*ce_gl_window_pos_2iv_proc)(v);
}

// point parameters

void ce_gl_point_parameter_f(GLenum pname, GLfloat param)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_point_parameter_f_proc);
	(*ce_gl_point_parameter_f_proc)(pname, param);
}

void ce_gl_point_parameter_fv(GLenum pname, GLfloat* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_point_parameter_fv_proc);
	(*ce_gl_point_parameter_fv_proc)(pname, params);
}

// occlusion query

void ce_gl_gen_queries(GLsizei n, GLuint* ids)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_gen_queries_proc);
	(*ce_gl_gen_queries_proc)(n, ids);
}

void ce_gl_delete_queries(GLsizei n, const GLuint* ids)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_delete_queries_proc);
	(*ce_gl_delete_queries_proc)(n, ids);
}

GLboolean ce_gl_is_query(GLuint id)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_is_query_proc);
	return (*ce_gl_is_query_proc)(id);
}

void ce_gl_begin_query(GLenum target, GLuint id)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_begin_query_proc);
	(*ce_gl_begin_query_proc)(target, id);
}

void ce_gl_end_query(GLenum target)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_end_query_proc);
	(*ce_gl_end_query_proc)(target);
}

void ce_gl_get_query_iv(GLenum target, GLenum pname, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_query_iv_proc);
	(*ce_gl_get_query_iv_proc)(target, pname, params);
}

void ce_gl_get_query_object_iv(GLuint id, GLenum pname, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_query_object_iv_proc);
	(ce_gl_get_query_object_iv_proc)(id, pname, params);
}

void ce_gl_get_query_object_uiv(GLuint id, GLenum pname, GLuint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_query_object_uiv_proc);
	(*ce_gl_get_query_object_uiv_proc)(id, pname, params);
}

// VBO

void ce_gl_bind_buffer(GLenum target, GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_bind_buffer_proc);
	(*ce_gl_bind_buffer_proc)(target, buffer);
}

void ce_gl_delete_buffers(GLsizei n, const GLuint* buffers)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_delete_buffers_proc);
	(*ce_gl_delete_buffers_proc)(n, buffers);
}

void ce_gl_gen_buffers(GLsizei n, GLuint* buffers)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_gen_buffers_proc);
	(*ce_gl_gen_buffers_proc)(n, buffers);
}

GLboolean ce_gl_is_buffer(GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_is_buffer_proc);
	return (*ce_gl_is_buffer_proc)(buffer);
}

void ce_gl_buffer_data(GLenum target, GLsizeiptr size,
						const GLvoid* data, GLenum usage)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_buffer_data_proc);
	(*ce_gl_buffer_data_proc)(target, size, data, usage);
}

void ce_gl_buffer_sub_data(GLenum target, GLintptr offset,
							GLsizeiptr size, const GLvoid* data)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_buffer_sub_data_proc);
	(*ce_gl_buffer_sub_data_proc)(target, offset, size, data);
}

void ce_gl_get_buffer_sub_data(GLenum target, GLintptr offset,
								GLsizeiptr size, GLvoid* data)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_buffer_sub_data_proc);
	(*ce_gl_get_buffer_sub_data_proc)(target, offset, size, data);
}

void* ce_gl_map_buffer(GLenum target, GLenum access)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_map_buffer_proc);
	return (*ce_gl_map_buffer_proc)(target, access);
}

GLboolean ce_gl_unmap_buffer(GLenum target)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_unmap_buffer_proc);
	return (*ce_gl_unmap_buffer_proc)(target);
}

void ce_gl_get_buffer_parameter_iv(GLenum target, GLenum pname, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_buffer_parameter_iv_proc);
	(*ce_gl_get_buffer_parameter_iv_proc)(target, pname, params);
}

void ce_gl_get_buffer_pointer_v(GLenum target, GLenum pname, GLvoid** params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_buffer_pointer_v_proc);
	(*ce_gl_get_buffer_pointer_v_proc)(target, pname, params);
}

void ce_gl_copy_buffer_sub_data(GLenum read_target, GLenum write_target,
	GLintptr read_offset, GLintptr write_offset, GLsizeiptr size)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_copy_buffer_sub_data_proc);
	(*ce_gl_copy_buffer_sub_data_proc)(read_target, write_target,
		read_offset, write_offset, size);
}

// FBO

GLboolean ce_gl_is_render_buffer(GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_is_render_buffer_proc);
	return (*ce_gl_is_render_buffer_proc)(buffer);
}

void ce_gl_bind_render_buffer(GLenum target, GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_bind_render_buffer_proc);
	(*ce_gl_bind_render_buffer_proc)(target, buffer);
}

void ce_gl_delete_render_buffers(GLsizei n, const GLuint* buffers)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_delete_render_buffers_proc);
	(*ce_gl_delete_render_buffers_proc)(n, buffers);
}

void ce_gl_gen_render_buffers(GLsizei n, GLuint* buffers)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_gen_render_buffers_proc);
	(*ce_gl_gen_render_buffers_proc)(n, buffers);
}

void ce_gl_render_buffer_storage(GLenum target, GLenum internal_format,
										GLsizei width, GLsizei height)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_render_buffer_storage_proc);
	(*ce_gl_render_buffer_storage_proc)(target, internal_format, width, height);
}

void ce_gl_render_buffer_storage_multisample(GLenum target, GLsizei samples,
											GLenum internal_format,
											GLsizei width, GLsizei height)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_render_buffer_storage_multisample_proc);
	(*ce_gl_render_buffer_storage_multisample_proc)
		(target, samples, internal_format, width, height);
}

void ce_gl_get_render_buffer_parameter_iv(GLenum target,
											GLenum pname, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_render_buffer_parameter_iv_proc);
	(*ce_gl_get_render_buffer_parameter_iv_proc)(target, pname, params);
}

GLboolean ce_gl_is_frame_buffer(GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_is_frame_buffer_proc);
	return (*ce_gl_is_frame_buffer_proc)(buffer);
}

void ce_gl_bind_frame_buffer(GLenum target, GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_bind_frame_buffer_proc);
	(*ce_gl_bind_frame_buffer_proc)(target, buffer);
}

void ce_gl_delete_frame_buffers(GLsizei n, const GLuint* buffers)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_delete_frame_buffers_proc);
	(*ce_gl_delete_frame_buffers_proc)(n, buffers);
}

void ce_gl_gen_frame_buffers(GLsizei n, GLuint* buffers)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_gen_frame_buffers_proc);
	(*ce_gl_gen_frame_buffers_proc)(n, buffers);
}

GLenum ce_gl_check_frame_buffer_status(GLenum target)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_check_frame_buffer_status_proc);
	return (*ce_gl_check_frame_buffer_status_proc)(target);
}

void ce_gl_frame_buffer_texture_1d(GLenum target, GLenum attachment,
							GLenum tex_target, GLuint texture, GLint level)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_frame_buffer_texture_1d_proc);
	(*ce_gl_frame_buffer_texture_1d_proc)(target, attachment, tex_target,
														texture, level);
}

void ce_gl_frame_buffer_texture_2d(GLenum target, GLenum attachment,
							GLenum tex_target, GLuint texture, GLint level)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_frame_buffer_texture_2d_proc);
	(*ce_gl_frame_buffer_texture_2d_proc)(target, attachment, tex_target,
														texture, level);
}

void ce_gl_frame_buffer_texture_3d(GLenum target, GLenum attachment,
									GLenum tex_target, GLuint texture,
									GLint level, GLint layer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_frame_buffer_texture_3d_proc);
	(*ce_gl_frame_buffer_texture_3d_proc)(target, attachment, tex_target,
													texture, level, layer);
}

void ce_gl_frame_buffer_texture_layer(GLenum target, GLenum attachment,
									GLuint texture, GLint level, GLint layer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_frame_buffer_texture_layer_proc);
	(*ce_gl_frame_buffer_texture_layer_proc)
		(target, attachment, texture, level, layer);
}

void ce_gl_frame_buffer_render_buffer(GLenum fb_target, GLenum attachment,
                                 		GLenum rb_target, GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_frame_buffer_render_buffer_proc);
	(*ce_gl_frame_buffer_render_buffer_proc)
		(fb_target, attachment, rb_target, buffer);
}

void ce_gl_get_frame_buffer_attachment_parameter_iv
	(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_frame_buffer_attachment_parameter_iv_proc);
	(*ce_gl_get_frame_buffer_attachment_parameter_iv_proc)
		(target, attachment, pname, params);
}

void ce_gl_blit_frame_buffer(GLint src_x0, GLint src_y0,
							GLint src_x1, GLint src_y1,
							GLint dst_x0, GLint dst_y0,
							GLint dst_x1, GLint dst_y1,
							GLbitfield mask, GLenum filter)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_blit_frame_buffer_proc);
	(*ce_gl_blit_frame_buffer_proc)(src_x0, src_y0,
		src_x1, src_y1, dst_x0, dst_y0, dst_x1, dst_y1, mask, filter);
}

void ce_gl_generate_mipmap(GLenum target)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_generate_mipmap_proc);
	(*ce_gl_generate_mipmap_proc)(target);
}

// TBO

void ce_gl_tex_buffer(GLenum target, GLenum internal_format, GLuint buffer)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_tex_buffer_proc);
	(*ce_gl_tex_buffer_proc)(target, internal_format, buffer);
}

// shader object

void ce_gl_delete_object(GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_delete_object_proc);
	(*ce_gl_delete_object_proc)(object);
}

GLhandle ce_gl_get_handle(GLenum pname)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_handle_proc);
	return (*ce_gl_get_handle_proc)(pname);
}

void ce_gl_detach_object(GLhandle container, GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_detach_object_proc);
	(*ce_gl_detach_object_proc)(container, object);
}

GLhandle ce_gl_create_shader_object(GLenum type)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_create_shader_object_proc);
	return (*ce_gl_create_shader_object_proc)(type);
}

void ce_gl_shader_source(GLhandle object, GLsizei count,
						const GLchar** string, const GLint* length)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_shader_source_proc);
	(*ce_gl_shader_source_proc)(object, count, string, length);
}

void ce_gl_compile_shader(GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_compile_shader_proc);
	(*ce_gl_compile_shader_proc)(object);
}

GLhandle ce_gl_create_program_object(void)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_create_program_object_proc);
	return (*ce_gl_create_program_object_proc)();
}

void ce_gl_attach_object(GLhandle container, GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_attach_object_proc);
	(*ce_gl_attach_object_proc)(container, object);
}

void ce_gl_link_program_object(GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_link_program_object_proc);
	(*ce_gl_link_program_object_proc)(object);
}

void ce_gl_use_program_object(GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_use_program_object_proc);
	(*ce_gl_use_program_object_proc)(object);
}

void ce_gl_validate_program_object(GLhandle object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_validate_program_object_proc);
	(*ce_gl_validate_program_object_proc)(object);
}

void ce_gl_uniform_1f(GLint location, GLfloat v0)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_1f_proc);
	(*ce_gl_uniform_1f_proc)(location, v0);
}

void ce_gl_uniform_2f(GLint location, GLfloat v0, GLfloat v1)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_2f_proc);
	(*ce_gl_uniform_2f_proc)(location, v0, v1);
}

void ce_gl_uniform_3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_3f_proc);
	(*ce_gl_uniform_3f_proc)(location, v0, v1, v2);
}

void ce_gl_uniform_4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_4f_proc);
	(*ce_gl_uniform_4f_proc)(location, v0, v1, v2, v3);
}

void ce_gl_uniform_1i(GLint location, GLint v0)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_1i_proc);
	(*ce_gl_uniform_1i_proc)(location, v0);
}

void ce_gl_uniform_2i(GLint location, GLint v0, GLint v1)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_2i_proc);
	(*ce_gl_uniform_2i_proc)(location, v0, v1);
}

void ce_gl_uniform_3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_3i_proc);
	(*ce_gl_uniform_3i_proc)(location, v0, v1, v2);
}

void ce_gl_uniform_4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_4i_proc);
	(*ce_gl_uniform_4i_proc)(location, v0, v1, v2, v3);
}

void ce_gl_uniform_1fv(GLint location, GLsizei count, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_1fv_proc);
	(*ce_gl_uniform_1fv_proc)(location, count, value);
}

void ce_gl_uniform_2fv(GLint location, GLsizei count, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_2fv_proc);
	(*ce_gl_uniform_2fv_proc)(location, count, value);
}

void ce_gl_uniform_3fv(GLint location, GLsizei count, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_3fv_proc);
	(*ce_gl_uniform_3fv_proc)(location, count, value);
}

void ce_gl_uniform_4fv(GLint location, GLsizei count, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_4fv_proc);
	(*ce_gl_uniform_4fv_proc)(location, count, value);
}

void ce_gl_uniform_1iv(GLint location, GLsizei count, const GLint* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_1iv_proc);
	(*ce_gl_uniform_1iv_proc)(location, count, value);
}

void ce_gl_uniform_2iv(GLint location, GLsizei count, const GLint* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_2iv_proc);
	(*ce_gl_uniform_2iv_proc)(location, count, value);
}

void ce_gl_uniform_3iv(GLint location, GLsizei count, const GLint* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_3iv_proc);
	(*ce_gl_uniform_3iv_proc)(location, count, value);
}

void ce_gl_uniform_4iv(GLint location, GLsizei count, const GLint* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_4iv_proc);
	(*ce_gl_uniform_4iv_proc)(location, count, value);
}

void ce_gl_uniform_matrix_2fv(GLint location, GLsizei count,
								GLboolean transpose, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_matrix_2fv_proc);
	(*ce_gl_uniform_matrix_2fv_proc)(location, count, transpose, value);
}

void ce_gl_uniform_matrix_3fv(GLint location, GLsizei count,
								GLboolean transpose, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_matrix_3fv_proc);
	(*ce_gl_uniform_matrix_3fv_proc)(location, count, transpose, value);
}

void ce_gl_uniform_matrix_4fv(GLint location, GLsizei count,
								GLboolean transpose, const GLfloat* value)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_uniform_matrix_4fv_proc);
	(*ce_gl_uniform_matrix_4fv_proc)(location, count, transpose, value);
}

void ce_gl_get_object_parameter_fv(GLhandle object, GLenum pname, GLfloat* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_object_parameter_fv_proc);
	(*ce_gl_get_object_parameter_fv_proc)(object, pname, params);
}

void ce_gl_get_object_parameter_iv(GLhandle object, GLenum pname, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_object_parameter_iv_proc);
	(*ce_gl_get_object_parameter_iv_proc)(object, pname, params);
}

void ce_gl_get_info_log(GLhandle object, GLsizei max_length,
						GLsizei* length, GLchar* info_log)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_info_log_proc);
	(*ce_gl_get_info_log_proc)(object, max_length, length, info_log);
}

void ce_gl_get_attached_objects(GLhandle container, GLsizei max_count,
								GLsizei* count, GLhandle* object)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_attached_objects_proc);
	(*ce_gl_get_attached_objects_proc)(container, max_count, count, object);
}

GLint ce_gl_get_uniform_location(GLhandle object, const GLchar* name)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_uniform_location_proc);
	return (*ce_gl_get_uniform_location_proc)(object, name);
}

void ce_gl_get_active_uniform(GLhandle object, GLuint index,
								GLsizei max_length, GLsizei* length,
								GLint* size, GLenum* type, GLchar* name)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_active_uniform_proc);
	(*ce_gl_get_active_uniform_proc)(object, index, max_length,
									length, size, type, name);
}

void ce_gl_get_uniform_fv(GLhandle object, GLint location, GLfloat* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_uniform_fv_proc);
	(*ce_gl_get_uniform_fv_proc)(object, location, params);
}

void ce_gl_get_uniform_iv(GLhandle object, GLint location, GLint* params)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_uniform_iv_proc);
	(*ce_gl_get_uniform_iv_proc)(object, location, params);
}

void ce_gl_get_shader_source(GLhandle object, GLsizei max_length,
								GLsizei* length, GLchar* source)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_shader_source_proc);
	(*ce_gl_get_shader_source_proc)(object, max_length, length, source);
}

// vertex shader tessellator

void ce_gl_vst_set_tessellation_factor(GLfloat factor)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_vst_set_tessellation_factor_proc);
	(*ce_gl_vst_set_tessellation_factor_proc)(factor);
}

void ce_gl_vst_set_tessellation_mode(GLenum mode)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_vst_set_tessellation_mode_proc);
	(*ce_gl_vst_set_tessellation_mode_proc)(mode);
}

// performance monitor

void ce_gl_get_perfmon_groups(GLint* count, GLsizei size, GLuint* groups)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_perfmon_groups_proc);
	(*ce_gl_get_perfmon_groups_proc)(count, size, groups);
}

void ce_gl_get_perfmon_counters(GLuint group, GLint* count, GLint* max_active,
								GLsizei size, GLuint* counters)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_perfmon_counters_proc);
	(*ce_gl_get_perfmon_counters_proc)(group, count, max_active, size, counters);
}

void ce_gl_get_perfmon_group_str(GLuint group, GLsizei size,
								GLsizei* length, GLchar* str)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_perfmon_group_str_proc);
	(*ce_gl_get_perfmon_group_str_proc)(group, size, length, str);
}

void ce_gl_get_perfmon_counter_str(GLuint group, GLuint counter,
									GLsizei size, GLsizei* length, GLchar* str)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_perfmon_counter_str_proc);
	(*ce_gl_get_perfmon_counter_str_proc)(group, counter, size, length, str);
}

void ce_gl_get_perfmon_counter_info(GLuint group, GLuint counter,
									GLenum pname, GLvoid* data)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_perfmon_counter_info_proc);
	(*ce_gl_get_perfmon_counter_info_proc)(group, counter, pname, data);
}

void ce_gl_gen_perfmons(GLsizei n, GLuint* monitors)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_gen_perfmons_proc);
	(*ce_gl_gen_perfmons_proc)(n, monitors);
}

void ce_gl_delete_perfmons(GLsizei n, GLuint* monitors)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_delete_perfmons_proc);
	(*ce_gl_delete_perfmons_proc)(n, monitors);
}

void ce_gl_select_perfmon_counters(GLuint monitor, GLboolean enable,
									GLuint group, GLint count, GLuint* list)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_select_perfmon_counters_proc);
	(*ce_gl_select_perfmon_counters_proc)(monitor, enable, group, count, list);
}

void ce_gl_begin_perfmon(GLuint monitor)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_begin_perfmon_proc);
	(*ce_gl_begin_perfmon_proc)(monitor);
}

void ce_gl_end_perfmon(GLuint monitor)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_end_perfmon_proc);
	(*ce_gl_end_perfmon_proc)(monitor);
}

void ce_gl_get_perfmon_counter_data(GLuint monitor, GLenum pname, GLsizei size,
									GLuint* data, GLint* bytes_written)
{
	CE_GL_CHECK_PROC_POINTER(ce_gl_get_perfmon_counter_data_proc);
	(*ce_gl_get_perfmon_counter_data_proc)(monitor, pname, size, data, bytes_written);
}
