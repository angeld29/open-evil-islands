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

#include <GL/gl.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	CE_GL_FEATURE_TEXTURE_RECTANGLE,
	CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1,
	CE_GL_FEATURE_TEXTURE_LOD,
	CE_GL_FEATURE_TEXTURE_EDGE_CLAMP,
	CE_GL_FEATURE_PACKED_PIXELS,
	CE_GL_FEATURE_GENERATE_MIPMAP,
	CE_GL_FEATURE_COUNT
} ce_gl_feature;

extern const GLenum CE_GL_COMPRESSED_RGB_S3TC_DXT1;
extern const GLenum CE_GL_COMPRESSED_RGBA_S3TC_DXT3;
extern const GLenum CE_GL_TEXTURE_MAX_LEVEL;
extern const GLenum CE_GL_CLAMP_TO_EDGE;
extern const GLenum CE_GL_UNSIGNED_SHORT_4_4_4_4;
extern const GLenum CE_GL_UNSIGNED_SHORT_5_5_5_1;
extern const GLenum CE_GL_UNSIGNED_INT_8_8_8_8;
extern const GLenum CE_GL_GENERATE_MIPMAP;

extern bool ce_gl_init(void);
extern void ce_gl_term(void);

extern bool ce_gl_report_errors(void);
extern bool ce_gl_query_feature(ce_gl_feature feature);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GL_H */
