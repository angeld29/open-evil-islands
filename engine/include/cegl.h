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

// GL types for handling large vertex buffer objects
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

// GL types for program/shader text and shader object handles
typedef char GLchar;
typedef unsigned int GLhandle;

// multitexture
extern const GLenum CE_GL_TEXTURE0;
extern const GLenum CE_GL_TEXTURE1;
extern const GLenum CE_GL_TEXTURE2;
extern const GLenum CE_GL_TEXTURE3;
extern const GLenum CE_GL_TEXTURE4;
extern const GLenum CE_GL_TEXTURE5;
extern const GLenum CE_GL_TEXTURE6;
extern const GLenum CE_GL_TEXTURE7;
extern const GLenum CE_GL_TEXTURE8;
extern const GLenum CE_GL_TEXTURE9;
extern const GLenum CE_GL_ACTIVE_TEXTURE;
extern const GLenum CE_GL_CLIENT_ACTIVE_TEXTURE;
extern const GLenum CE_GL_MAX_TEXTURE_UNITS;

extern void ce_gl_active_texture(GLenum target);
extern void ce_gl_client_active_texture(GLenum target);
extern void ce_gl_multi_tex_coord_2f(GLenum target, GLfloat s, GLfloat t);
extern void ce_gl_multi_tex_coord_2fv(GLenum target, const GLfloat* v);
extern void ce_gl_multi_tex_coord_3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void ce_gl_multi_tex_coord_3fv(GLenum target, const GLfloat* v);

// texture compression
extern const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT1;
extern const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3;

extern void ce_gl_compressed_tex_image_2d(GLenum target, GLint level,
											GLenum internal_format,
											GLsizei width, GLsizei height,
											GLint border, GLsizei image_size,
											const GLvoid* data);

// texture lod
extern const GLenum CE_GL_TEXTURE_MAX_LEVEL;

// texture clamp
extern const GLenum CE_GL_CLAMP_TO_BORDER;
extern const GLenum CE_GL_CLAMP_TO_EDGE;

// texture float
extern const GLenum CE_GL_TEXTURE_RED_TYPE;
extern const GLenum CE_GL_TEXTURE_GREEN_TYPE;
extern const GLenum CE_GL_TEXTURE_BLUE_TYPE;
extern const GLenum CE_GL_TEXTURE_ALPHA_TYPE;
extern const GLenum CE_GL_RGBA32F;
extern const GLenum CE_GL_RGB32F;
extern const GLenum CE_GL_ALPHA32F;
extern const GLenum CE_GL_RGBA16F;
extern const GLenum CE_GL_RGB16F;
extern const GLenum CE_GL_ALPHA16F;

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

// window pos
extern void ce_gl_window_pos_2f(GLfloat x, GLfloat y);
extern void ce_gl_window_pos_2fv(const GLfloat* v);
extern void ce_gl_window_pos_2i(GLint x, GLint y);
extern void ce_gl_window_pos_2iv(const GLint* v);

// point parameters
extern const GLenum CE_GL_POINT_SIZE_MIN;
extern const GLenum CE_GL_POINT_SIZE_MAX;
extern const GLenum CE_GL_POINT_FADE_THRESHOLD_SIZE;
extern const GLenum CE_GL_POINT_DISTANCE_ATTENUATION;

extern void ce_gl_point_parameter_f(GLenum pname, GLfloat param);
extern void ce_gl_point_parameter_fv(GLenum pname, GLfloat* params);

// point sprite
extern const GLenum CE_GL_POINT_SPRITE;
extern const GLenum CE_GL_COORD_REPLACE;

// occlusion query
extern const GLenum CE_GL_SAMPLES_PASSED;
extern const GLenum CE_GL_ANY_SAMPLES_PASSED;
extern const GLenum CE_GL_QUERY_COUNTER_BITS;
extern const GLenum CE_GL_CURRENT_QUERY;
extern const GLenum CE_GL_QUERY_RESULT;
extern const GLenum CE_GL_QUERY_RESULT_AVAILABLE;

extern void ce_gl_gen_queries(GLsizei n, GLuint* ids);
extern void ce_gl_delete_queries(GLsizei n, const GLuint* ids);
extern GLboolean ce_gl_is_query(GLuint id);
extern void ce_gl_begin_query(GLenum target, GLuint id);
extern void ce_gl_end_query(GLenum target);
extern void ce_gl_get_query_iv(GLenum target, GLenum pname, GLint* params);
extern void ce_gl_get_query_object_iv(GLuint id, GLenum pname, GLint* params);
extern void ce_gl_get_query_object_uiv(GLuint id, GLenum pname, GLuint* params);

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

// VBO
extern const GLenum CE_GL_ARRAY_BUFFER;
extern const GLenum CE_GL_ELEMENT_ARRAY_BUFFER;
extern const GLenum CE_GL_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_ELEMENT_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_VERTEX_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_NORMAL_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_COLOR_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_INDEX_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_EDGE_FLAG_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_WEIGHT_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING;
extern const GLenum CE_GL_STREAM_DRAW;
extern const GLenum CE_GL_STREAM_READ;
extern const GLenum CE_GL_STREAM_COPY;
extern const GLenum CE_GL_STATIC_DRAW;
extern const GLenum CE_GL_STATIC_READ;
extern const GLenum CE_GL_STATIC_COPY;
extern const GLenum CE_GL_DYNAMIC_DRAW;
extern const GLenum CE_GL_DYNAMIC_READ;
extern const GLenum CE_GL_DYNAMIC_COPY;
extern const GLenum CE_GL_READ_ONLY;
extern const GLenum CE_GL_WRITE_ONLY;
extern const GLenum CE_GL_READ_WRITE;
extern const GLenum CE_GL_BUFFER_SIZE;
extern const GLenum CE_GL_BUFFER_USAGE;
extern const GLenum CE_GL_BUFFER_ACCESS;
extern const GLenum CE_GL_BUFFER_MAPPED;
extern const GLenum CE_GL_BUFFER_MAP_POINTER;
extern const GLenum CE_GL_COPY_READ_BUFFER;
extern const GLenum CE_GL_COPY_WRITE_BUFFER;

extern void ce_gl_bind_buffer(GLenum target, GLuint buffer);
extern void ce_gl_delete_buffers(GLsizei n, const GLuint* buffers);
extern void ce_gl_gen_buffers(GLsizei n, GLuint* buffers);
extern GLboolean ce_gl_is_buffer(GLuint buffer);
extern void ce_gl_buffer_data(GLenum target, GLsizeiptr size,
								const GLvoid* data, GLenum usage);
extern void ce_gl_buffer_sub_data(GLenum target, GLintptr offset,
									GLsizeiptr size, const GLvoid* data);
extern void ce_gl_get_buffer_sub_data(GLenum target, GLintptr offset,
									GLsizeiptr size, GLvoid* data);
extern void* ce_gl_map_buffer(GLenum target, GLenum access);
extern GLboolean ce_gl_unmap_buffer(GLenum target);
extern void ce_gl_get_buffer_parameter_iv(GLenum target, GLenum pname, GLint* params);
extern void ce_gl_get_buffer_pointer_v(GLenum target, GLenum pname, GLvoid** params);
extern void ce_gl_copy_buffer_sub_data(GLenum read_target, GLenum write_target,
	GLintptr read_offset, GLintptr write_offset, GLsizeiptr size);

// FBO
extern const GLenum CE_GL_FRAME_BUFFER;
extern const GLenum CE_GL_READ_FRAME_BUFFER;
extern const GLenum CE_GL_DRAW_FRAME_BUFFER;
extern const GLenum CE_GL_RENDER_BUFFER;
extern const GLenum CE_GL_STENCIL_INDEX1;
extern const GLenum CE_GL_STENCIL_INDEX4;
extern const GLenum CE_GL_STENCIL_INDEX8;
extern const GLenum CE_GL_STENCIL_INDEX16;
extern const GLenum CE_GL_RENDER_BUFFER_WIDTH;
extern const GLenum CE_GL_RENDER_BUFFER_HEIGHT;
extern const GLenum CE_GL_RENDER_BUFFER_INTERNAL_FORMAT;
extern const GLenum CE_GL_RENDER_BUFFER_RED_SIZE;
extern const GLenum CE_GL_RENDER_BUFFER_GREEN_SIZE;
extern const GLenum CE_GL_RENDER_BUFFER_BLUE_SIZE;
extern const GLenum CE_GL_RENDER_BUFFER_ALPHA_SIZE;
extern const GLenum CE_GL_RENDER_BUFFER_DEPTH_SIZE;
extern const GLenum CE_GL_RENDER_BUFFER_STENCIL_SIZE;
extern const GLenum CE_GL_RENDER_BUFFER_SAMPLES;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_OBJECT_TYPE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_OBJECT_NAME;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_TEXTURE_LEVEL;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_TEXTURE_LAYER;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_COLOR_ENCODING;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_COMPONENT_TYPE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_RED_SIZE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_GREEN_SIZE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_BLUE_SIZE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_ALPHA_SIZE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_DEPTH_SIZE;
extern const GLenum CE_GL_FRAME_BUFFER_ATTACHMENT_STENCIL_SIZE;
extern const GLenum CE_GL_COLOR_ATTACHMENT0;
extern const GLenum CE_GL_COLOR_ATTACHMENT1;
extern const GLenum CE_GL_COLOR_ATTACHMENT2;
extern const GLenum CE_GL_COLOR_ATTACHMENT3;
extern const GLenum CE_GL_COLOR_ATTACHMENT4;
extern const GLenum CE_GL_COLOR_ATTACHMENT5;
extern const GLenum CE_GL_COLOR_ATTACHMENT6;
extern const GLenum CE_GL_COLOR_ATTACHMENT7;
extern const GLenum CE_GL_COLOR_ATTACHMENT8;
extern const GLenum CE_GL_COLOR_ATTACHMENT9;
extern const GLenum CE_GL_DEPTH_ATTACHMENT;
extern const GLenum CE_GL_STENCIL_ATTACHMENT;
extern const GLenum CE_GL_DEPTH_STENCIL_ATTACHMENT;
extern const GLenum CE_GL_FRAME_BUFFER_COMPLETE;
extern const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_ATTACHMENT;
extern const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_MISSING_ATTACHMENT;
extern const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_DRAW_BUFFER;
extern const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_READ_BUFFER;
extern const GLenum CE_GL_FRAME_BUFFER_UNSUPPORTED;
extern const GLenum CE_GL_FRAME_BUFFER_INCOMPLETE_MULTISAMPLE;
extern const GLenum CE_GL_FRAME_BUFFER_UNDEFINED;
extern const GLenum CE_GL_FRAME_BUFFER_BINDING;
extern const GLenum CE_GL_DRAW_FRAME_BUFFER_BINDING;
extern const GLenum CE_GL_READ_FRAME_BUFFER_BINDING;
extern const GLenum CE_GL_RENDER_BUFFER_BINDING;
extern const GLenum CE_GL_MAX_COLOR_ATTACHMENTS;
extern const GLenum CE_GL_MAX_RENDER_BUFFER_SIZE;
extern const GLenum CE_GL_INVALID_FRAME_BUFFER_OPERATION;

extern GLboolean ce_gl_is_render_buffer(GLuint buffer);
extern void ce_gl_bind_render_buffer(GLenum target, GLuint buffer);
extern void ce_gl_delete_render_buffers(GLsizei n, const GLuint* buffers);
extern void ce_gl_gen_render_buffers(GLsizei n, GLuint* buffers);
extern void ce_gl_render_buffer_storage(GLenum target, GLenum internal_format,
										GLsizei width, GLsizei height);
extern void ce_gl_render_buffer_storage_multisample(GLenum target,
	GLsizei samples, GLenum internal_format, GLsizei width, GLsizei height);
extern void ce_gl_get_render_buffer_parameter_iv(GLenum target,
												GLenum pname, GLint* params);
extern GLboolean ce_gl_is_frame_buffer(GLuint buffer);
extern void ce_gl_bind_frame_buffer(GLenum target, GLuint buffer);
extern void ce_gl_delete_frame_buffers(GLsizei n, const GLuint* buffers);
extern void ce_gl_gen_frame_buffers(GLsizei n, GLuint* buffers);
extern GLenum ce_gl_check_frame_buffer_status(GLenum target);
extern void ce_gl_frame_buffer_texture_1d(GLenum target, GLenum attachment,
							GLenum tex_target, GLuint texture, GLint level);
extern void ce_gl_frame_buffer_texture_2d(GLenum target, GLenum attachment,
							GLenum tex_target, GLuint texture, GLint level);
extern void ce_gl_frame_buffer_texture_3d(GLenum target, GLenum attachment,
										GLenum tex_target, GLuint texture,
										GLint level, GLint layer);
extern void ce_gl_frame_buffer_texture_layer(GLenum target, GLenum attachment,
									GLuint texture, GLint level, GLint layer);
extern void ce_gl_frame_buffer_render_buffer(GLenum fb_target, GLenum attachment,
                                 			GLenum rb_target, GLuint buffer);
extern void ce_gl_get_frame_buffer_attachment_parameter_iv(GLenum target,
								GLenum attachment, GLenum pname, GLint* params);
extern void ce_gl_blit_frame_buffer(GLint src_x0, GLint src_y0,
									GLint src_x1, GLint src_y1,
									GLint dst_x0, GLint dst_y0,
									GLint dst_x1, GLint dst_y1,
									GLbitfield mask, GLenum filter);
extern void ce_gl_generate_mipmap(GLenum target);

// PBO
extern const GLenum CE_GL_PIXEL_PACK_BUFFER;
extern const GLenum CE_GL_PIXEL_UNPACK_BUFFER;
extern const GLenum CE_GL_PIXEL_PACK_BUFFER_BINDING;
extern const GLenum CE_GL_PIXEL_UNPACK_BUFFER_BINDING;

// TBO
extern const GLenum CE_GL_TEXTURE_BUFFER;
extern const GLenum CE_GL_MAX_TEXTURE_BUFFER_SIZE;
extern const GLenum CE_GL_TEXTURE_BINDING_BUFFER;
extern const GLenum CE_GL_TEXTURE_BUFFER_DATA_STORE_BINDING;
extern const GLenum CE_GL_TEXTURE_BUFFER_FORMAT;

extern void ce_gl_tex_buffer(GLenum target, GLenum internal_format, GLuint buffer);

// shading language
extern const GLenum CE_GL_SHADING_LANGUAGE_VERSION;

// shader object
extern const GLenum CE_GL_PROGRAM_OBJECT;
extern const GLenum CE_GL_OBJECT_TYPE;
extern const GLenum CE_GL_OBJECT_SUBTYPE;
extern const GLenum CE_GL_OBJECT_DELETE_STATUS;
extern const GLenum CE_GL_OBJECT_COMPILE_STATUS;
extern const GLenum CE_GL_OBJECT_LINK_STATUS;
extern const GLenum CE_GL_OBJECT_VALIDATE_STATUS;
extern const GLenum CE_GL_OBJECT_INFO_LOG_LENGTH;
extern const GLenum CE_GL_OBJECT_ATTACHED_OBJECTS;
extern const GLenum CE_GL_OBJECT_ACTIVE_UNIFORMS;
extern const GLenum CE_GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH;
extern const GLenum CE_GL_OBJECT_SHADER_SOURCE_LENGTH;
extern const GLenum CE_GL_SHADER_OBJECT;

extern void ce_gl_delete_object(GLhandle object);
extern GLhandle ce_gl_get_handle(GLenum pname);
extern void ce_gl_detach_object(GLhandle container, GLhandle object);
extern GLhandle ce_gl_create_shader_object(GLenum type);
extern void ce_gl_shader_source(GLhandle object, GLsizei count,
								const GLchar** string, const GLint* length);
extern void ce_gl_compile_shader(GLhandle object);
extern GLhandle ce_gl_create_program_object(void);
extern void ce_gl_attach_object(GLhandle container, GLhandle object);
extern void ce_gl_link_program_object(GLhandle object);
extern void ce_gl_use_program_object(GLhandle object);
extern void ce_gl_validate_program_object(GLhandle object);
extern void ce_gl_uniform_1f(GLint location, GLfloat v0);
extern void ce_gl_uniform_2f(GLint location, GLfloat v0, GLfloat v1);
extern void ce_gl_uniform_3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void ce_gl_uniform_4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void ce_gl_uniform_1i(GLint location, GLint v0);
extern void ce_gl_uniform_2i(GLint location, GLint v0, GLint v1);
extern void ce_gl_uniform_3i(GLint location, GLint v0, GLint v1, GLint v2);
extern void ce_gl_uniform_4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void ce_gl_uniform_1fv(GLint location, GLsizei count, const GLfloat* value);
extern void ce_gl_uniform_2fv(GLint location, GLsizei count, const GLfloat* value);
extern void ce_gl_uniform_3fv(GLint location, GLsizei count, const GLfloat* value);
extern void ce_gl_uniform_4fv(GLint location, GLsizei count, const GLfloat* value);
extern void ce_gl_uniform_1iv(GLint location, GLsizei count, const GLint* value);
extern void ce_gl_uniform_2iv(GLint location, GLsizei count, const GLint* value);
extern void ce_gl_uniform_3iv(GLint location, GLsizei count, const GLint* value);
extern void ce_gl_uniform_4iv(GLint location, GLsizei count, const GLint* value);
extern void ce_gl_uniform_matrix_2fv(GLint location, GLsizei count,
									GLboolean transpose, const GLfloat* value);
extern void ce_gl_uniform_matrix_3fv(GLint location, GLsizei count,
									GLboolean transpose, const GLfloat* value);
extern void ce_gl_uniform_matrix_4fv(GLint location, GLsizei count,
									GLboolean transpose, const GLfloat* value);
extern void ce_gl_get_object_parameter_fv(GLhandle object, GLenum pname, GLfloat* params);
extern void ce_gl_get_object_parameter_iv(GLhandle object, GLenum pname, GLint* params);
extern void ce_gl_get_info_log(GLhandle object, GLsizei max_length,
								GLsizei* length, GLchar* info_log);
extern void ce_gl_get_attached_objects(GLhandle container, GLsizei max_count,
										GLsizei* count, GLhandle* object);
extern GLint ce_gl_get_uniform_location(GLhandle object, const GLchar* name);
extern void ce_gl_get_active_uniform(GLhandle object, GLuint index,
									GLsizei max_length, GLsizei* length,
									GLint* size, GLenum* type, GLchar* name);
extern void ce_gl_get_uniform_fv(GLhandle object, GLint location, GLfloat* params);
extern void ce_gl_get_uniform_iv(GLhandle object, GLint location, GLint* params);
extern void ce_gl_get_shader_source(GLhandle object, GLsizei max_length,
									GLsizei* length, GLchar* source);

// vertex shader
extern const GLenum CE_GL_VERTEX_SHADER;
extern const GLenum CE_GL_MAX_VERTEX_UNIFORM_COMPONENTS;
extern const GLenum CE_GL_MAX_VARYING_FLOATS;
extern const GLenum CE_GL_MAX_VERTEX_ATTRIBS;
extern const GLenum CE_GL_MAX_TEXTURE_IMAGE_UNITS;
extern const GLenum CE_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS;
extern const GLenum CE_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
extern const GLenum CE_GL_MAX_TEXTURE_COORDS;
extern const GLenum CE_GL_VERTEX_PROGRAM_POINT_SIZE;
extern const GLenum CE_GL_VERTEX_PROGRAM_TWO_SIDE;
extern const GLenum CE_GL_OBJECT_ACTIVE_ATTRIBUTES;
extern const GLenum CE_GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_ENABLED;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_SIZE;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_STRIDE;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_TYPE;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_NORMALIZED;
extern const GLenum CE_GL_CURRENT_VERTEX_ATTRIB;
extern const GLenum CE_GL_VERTEX_ATTRIB_ARRAY_POINTER;

// fragment shader
extern const GLenum CE_GL_FRAGMENT_SHADER;
extern const GLenum CE_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS;
extern const GLenum CE_GL_FRAGMENT_SHADER_DERIVATIVE_HINT;

// vertex shader tessellator
extern const GLenum CE_GL_VST_SAMPLER_BUFFER;
extern const GLenum CE_GL_VST_INT_SAMPLER_BUFFER;
extern const GLenum CE_GL_VST_UNSIGNED_INT_SAMPLER_BUFFER;
extern const GLenum CE_GL_VST_DISCRETE;
extern const GLenum CE_GL_VST_CONTINUOUS;
extern const GLenum CE_GL_VST_TESSELLATION_MODE;
extern const GLenum CE_GL_VST_TESSELLATION_FACTOR;

extern void ce_gl_vst_set_tessellation_factor(GLfloat factor);
extern void ce_gl_vst_set_tessellation_mode(GLenum mode);

// performance monitor
extern const GLenum CE_GL_PERFMON_COUNTER_TYPE;
extern const GLenum CE_GL_PERFMON_COUNTER_RANGE;
extern const GLenum CE_GL_PERFMON_UNSIGNED_INT64;
extern const GLenum CE_GL_PERFMON_PERCENTAGE;
extern const GLenum CE_GL_PERFMON_RESULT_AVAILABLE;
extern const GLenum CE_GL_PERFMON_RESULT_SIZE;
extern const GLenum CE_GL_PERFMON_RESULT;

extern void ce_gl_get_perfmon_groups(GLint* count, GLsizei size, GLuint* groups);
extern void ce_gl_get_perfmon_counters(GLuint group, GLint* count,
										GLint* max_active, GLsizei size,
										GLuint* counters);
extern void ce_gl_get_perfmon_group_str(GLuint group, GLsizei size,
										GLsizei* length, GLchar* str);
extern void ce_gl_get_perfmon_counter_str(GLuint group, GLuint counter,
										GLsizei size, GLsizei* length, GLchar* str);
extern void ce_gl_get_perfmon_counter_info(GLuint group, GLuint counter,
											GLenum pname, GLvoid* data);
extern void ce_gl_gen_perfmons(GLsizei n, GLuint* monitors);
extern void ce_gl_delete_perfmons(GLsizei n, GLuint* monitors);
extern void ce_gl_select_perfmon_counters(GLuint monitor, GLboolean enable,
										GLuint group, GLint count, GLuint* list);
extern void ce_gl_begin_perfmon(GLuint monitor);
extern void ce_gl_end_perfmon(GLuint monitor);
extern void ce_gl_get_perfmon_counter_data(GLuint monitor, GLenum pname,
											GLsizei size, GLuint* data,
											GLint* bytes_written);

// meminfo
extern const GLenum CE_GL_VBO_FREE_MEMORY;
extern const GLenum CE_GL_TEXTURE_FREE_MEMORY;
extern const GLenum CE_GL_RENDERBUFFER_FREE_MEMORY;

// common API
typedef enum {
	CE_GL_FEATURE_MULTITEXTURE,
	CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	CE_GL_FEATURE_TEXTURE_RECTANGLE,
	CE_GL_FEATURE_TEXTURE_COMPRESSION,
	CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1,
	CE_GL_FEATURE_TEXTURE_LOD,
	CE_GL_FEATURE_TEXTURE_BORDER_CLAMP,
	CE_GL_FEATURE_TEXTURE_EDGE_CLAMP,
	CE_GL_FEATURE_TEXTURE_FLOAT,
	CE_GL_FEATURE_ABGR,
	CE_GL_FEATURE_BGRA,
	CE_GL_FEATURE_PACKED_PIXELS,
	CE_GL_FEATURE_GENERATE_MIPMAP,
	CE_GL_FEATURE_WINDOW_POS,
	CE_GL_FEATURE_POINT_PARAMETERS,
	CE_GL_FEATURE_POINT_SPRITE,
	CE_GL_FEATURE_OCCLUSION_QUERY,
	CE_GL_FEATURE_OCCLUSION_QUERY2,
	CE_GL_FEATURE_MULTISAMPLE,
	CE_GL_FEATURE_VERTEX_BUFFER_OBJECT,
	CE_GL_FEATURE_COPY_BUFFER,
	CE_GL_FEATURE_FRAME_BUFFER_OBJECT,
	CE_GL_FEATURE_PIXEL_BUFFER_OBJECT,
	CE_GL_FEATURE_TEXTURE_BUFFER_OBJECT,
	CE_GL_FEATURE_SHADING_LANGUAGE_100,
	CE_GL_FEATURE_SHADER_OBJECT,
	CE_GL_FEATURE_VERTEX_SHADER,
	CE_GL_FEATURE_FRAGMENT_SHADER,
	CE_GL_FEATURE_VERTEX_SHADER_TESSELLATOR,
	CE_GL_FEATURE_PERFORMANCE_MONITOR,
	CE_GL_FEATURE_MEMINFO,
	CE_GL_FEATURE_COUNT
} ce_gl_feature;

extern bool ce_gl_init(void);
extern void ce_gl_term(void);

extern bool ce_gl_report_errors(void);
extern bool ce_gl_query_feature(ce_gl_feature feature);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GL_H */
