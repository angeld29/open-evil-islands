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

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CEGL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	CEGL_FEATURE_TEXTURE_RECTANGLE,
	CEGL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	CEGL_FEATURE_TEXTURE_COMPRESSION_DXT1,
	CEGL_FEATURE_TEXTURE_LOD,
	CEGL_FEATURE_TEXTURE_EDGE_CLAMP,
	CEGL_FEATURE_COUNT
} cegl_feature;

extern const GLenum CEGL_COMPRESSED_RGB_S3TC_DXT1;
extern const GLenum CEGL_COMPRESSED_RGBA_S3TC_DXT3;
extern const GLenum CEGL_TEXTURE_MAX_LEVEL;
extern const GLenum CEGL_CLAMP_TO_EDGE;

extern void cegl_init(void);
extern bool cegl_report_errors(void);

extern bool cegl_query_feature(cegl_feature feature);
extern int cegl_max_texture_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GL_H */
