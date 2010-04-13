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

#ifndef CE_GL_H
#define CE_GL_H

#include <stddef.h>
#include <stdbool.h>

#include <GL/gl.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef GL_VERSION_1_5
// gl types for handling large vertex buffer objects
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
#endif

typedef enum {
	CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	CE_GL_FEATURE_TEXTURE_RECTANGLE,
	CE_GL_FEATURE_TEXTURE_COMPRESSION,
	CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1,
	CE_GL_FEATURE_TEXTURE_LOD,
	CE_GL_FEATURE_TEXTURE_BORDER_CLAMP,
	CE_GL_FEATURE_TEXTURE_EDGE_CLAMP,
	CE_GL_FEATURE_ABGR,
	CE_GL_FEATURE_BGRA,
	CE_GL_FEATURE_PACKED_PIXELS,
	CE_GL_FEATURE_GENERATE_MIPMAP,
	CE_GL_FEATURE_VERTEX_BUFFER_OBJECT,
	CE_GL_FEATURE_WINDOW_POS,
	CE_GL_FEATURE_POINT_PARAMETERS,
	CE_GL_FEATURE_POINT_SPRITE,
	CE_GL_FEATURE_MEMINFO,
	CE_GL_FEATURE_MULTISAMPLE,
	CE_GL_FEATURE_FRAMEBUFFER_OBJECT,
	CE_GL_FEATURE_PIXEL_BUFFER_OBJECT,
	CE_GL_FEATURE_TEXTURE_BUFFER_OBJECT,
	CE_GL_FEATURE_SHADING_LANGUAGE_100,
	CE_GL_FEATURE_COUNT
} ce_gl_feature;

// texture compression
extern const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT1;
extern const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3;

// texture lod
extern const GLenum CE_GL_TEXTURE_MAX_LEVEL;

// texture clamp
extern const GLenum CE_GL_CLAMP_TO_BORDER;
extern const GLenum CE_GL_CLAMP_TO_EDGE;

// color formats
extern const GLenum CE_GL_ABGR;
extern const GLenum CE_GL_BGR;
extern const GLenum CE_GL_BGRA;

// packed pixels
extern const GLenum CE_GL_UNSIGNED_SHORT_4_4_4_4;
extern const GLenum CE_GL_UNSIGNED_SHORT_5_5_5_1;
extern const GLenum CE_GL_UNSIGNED_INT_8_8_8_8;

// generate mipmap
extern const GLenum CE_GL_GENERATE_MIPMAP;

// VBO
extern const GLenum CE_GL_ARRAY_BUFFER;
extern const GLenum CE_GL_ELEMENT_ARRAY_BUFFER;
extern const GLenum CE_GL_STREAM_DRAW;
extern const GLenum CE_GL_STREAM_READ;
extern const GLenum CE_GL_STREAM_COPY;
extern const GLenum CE_GL_STATIC_DRAW;
extern const GLenum CE_GL_STATIC_READ;
extern const GLenum CE_GL_STATIC_COPY;
extern const GLenum CE_GL_DYNAMIC_DRAW;
extern const GLenum CE_GL_DYNAMIC_READ;
extern const GLenum CE_GL_DYNAMIC_COPY;

// point parameters
extern const GLenum CE_GL_POINT_SIZE_MIN;
extern const GLenum CE_GL_POINT_SIZE_MAX;
extern const GLenum CE_GL_POINT_FADE_THRESHOLD_SIZE;
extern const GLenum CE_GL_POINT_DISTANCE_ATTENUATION;

// point sprite
extern const GLenum CE_GL_POINT_SPRITE;
extern const GLenum CE_GL_COORD_REPLACE;

// meminfo
extern const GLenum CE_GL_VBO_FREE_MEMORY;
extern const GLenum CE_GL_TEXTURE_FREE_MEMORY;
extern const GLenum CE_GL_RENDERBUFFER_FREE_MEMORY;

// multisample
extern const GLenum CE_GLX_SAMPLE_BUFFERS;
extern const GLenum CE_GLX_SAMPLES;
extern const GLenum CE_WGL_SAMPLE_BUFFERS;
extern const GLenum CE_WGL_SAMPLES;
extern const GLenum CE_GL_MULTISAMPLE;
extern const GLenum CE_GL_SAMPLE_ALPHA_TO_COVERAGE;
extern const GLenum CE_GL_SAMPLE_ALPHA_TO_ONE;
extern const GLenum CE_GL_SAMPLE_COVERAGE;
extern const GLenum CE_GL_MULTISAMPLE_BIT;
extern const GLenum CE_GL_SAMPLE_BUFFERS;
extern const GLenum CE_GL_SAMPLES;
extern const GLenum CE_GL_SAMPLE_COVERAGE_VALUE;
extern const GLenum CE_GL_SAMPLE_COVERAGE_INVERT;

// FBO
extern const GLenum CE_GL_FRAMEBUFFER;
extern const GLenum CE_GL_READ_FRAMEBUFFER;
extern const GLenum CE_GL_DRAW_FRAMEBUFFER;
extern const GLenum CE_GL_COLOR_ATTACHMENT0;
extern const GLenum CE_GL_COLOR_ATTACHMENT1;
extern const GLenum CE_GL_FRAMEBUFFER_COMPLETE;

// PBO
extern const GLenum CE_GL_PIXEL_PACK_BUFFER;
extern const GLenum CE_GL_PIXEL_UNPACK_BUFFER;

// shading language
extern const GLenum CE_GL_SHADING_LANGUAGE_VERSION;

// common API
extern bool ce_gl_init(void);
extern void ce_gl_term(void);

extern bool ce_gl_report_errors(void);
extern bool ce_gl_query_feature(ce_gl_feature feature);

// texture compression
extern void ce_gl_compressed_tex_image_2d(GLenum target, GLint level,
											GLenum internal_format,
											GLsizei width, GLsizei height,
											GLint border, GLsizei image_size,
											const GLvoid* data);

// VBO
extern void ce_gl_bind_buffer(GLenum target, GLuint buffer);
extern void ce_gl_delete_buffers(GLsizei n, const GLuint* buffers);
extern void ce_gl_gen_buffers(GLsizei n, GLuint* buffers);
extern void ce_gl_buffer_data(GLenum target, GLsizeiptr size,
								const GLvoid* data, GLenum usage);
extern void ce_gl_buffer_sub_data(GLenum target, GLintptr offset,
									GLsizeiptr size, const GLvoid* data);

// window pos
extern void ce_gl_window_pos_2f(GLfloat x, GLfloat y);
extern void ce_gl_window_pos_2fv(const GLfloat* v);
extern void ce_gl_window_pos_2i(GLint x, GLint y);
extern void ce_gl_window_pos_2iv(const GLint* v);

// point parameters
extern void ce_gl_point_parameter_f(GLenum pname, GLfloat param);
extern void ce_gl_point_parameter_fv(GLenum pname, GLfloat* params);

// FBO
extern void ce_gl_bind_framebuffer(GLenum target, GLuint framebuffer);
extern void ce_gl_delete_framebuffers(GLsizei n, const GLuint* framebuffers);
extern void ce_gl_gen_framebuffers(GLsizei n, GLuint* framebuffers);
extern GLenum ce_gl_check_framebuffer_status(GLenum target);
extern void ce_gl_framebuffer_texture_2d(GLenum target, GLenum attachment,
							GLenum textarget, GLuint texture, GLint level);
extern void ce_gl_generate_mipmap(GLenum target);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GL_H */
