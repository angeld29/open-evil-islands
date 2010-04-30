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

#include <stdbool.h>

#include <GL/glew.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_GL_FEATURE_MULTITEXTURE,
	CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
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
	CE_GL_FEATURE_MAP_BUFFER_RANGE,
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
