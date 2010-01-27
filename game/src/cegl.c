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

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif

#ifndef GL_TEXTURE_MAX_LEVEL_SGIS
#define GL_TEXTURE_MAX_LEVEL_SGIS 0x813D
#endif

#include "logging.h"
#include "cegl.h"

const GLenum CEGL_COMPRESSED_RGB_S3TC_DXT1 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
const GLenum CEGL_COMPRESSED_RGBA_S3TC_DXT3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
const GLenum CEGL_TEXTURE_MAX_LEVEL = GL_TEXTURE_MAX_LEVEL_SGIS;

static bool features[CEGL_FEATURE_COUNT];

static bool check_extension(const char* name)
{
#ifdef GLU_VERSION_1_3
	return gluCheckExtension((const GLubyte*)name, glGetString(GL_EXTENSIONS));
#else
	return NULL != strstr((const char*)glGetString(GL_EXTENSIONS), name);
#endif
}

static void report_extension(const char* name, bool ok)
{
	logging_write("Checking for '%s' extension... %s.", name, ok ? "yes" : "no");
}

void cegl_init(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	features[CEGL_FEATURE_TEXTURE_NON_POWER_OF_TWO] =
		check_extension("GL_ARB_texture_non_power_of_two");
	features[CEGL_FEATURE_TEXTURE_RECTANGLE] =
		check_extension("GL_ARB_texture_rectangle") ||
		check_extension("GL_EXT_texture_rectangle") ||
		check_extension("GL_NV_texture_rectangle");
	features[CEGL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
		check_extension("GL_EXT_texture_compression_s3tc");
	features[CEGL_FEATURE_TEXTURE_COMPRESSION_DXT1] =
		check_extension("GL_EXT_texture_compression_dxt1");
	features[CEGL_FEATURE_TEXTURE_LOD] =
		check_extension("GL_SGIS_texture_lod") ||
		check_extension("GL_EXT_texture_lod");

	report_extension("texture non power of two",
		features[CEGL_FEATURE_TEXTURE_NON_POWER_OF_TWO]);
	report_extension("texture rectangle",
		features[CEGL_FEATURE_TEXTURE_RECTANGLE]);
	report_extension("texture compression s3tc",
		features[CEGL_FEATURE_TEXTURE_COMPRESSION_S3TC]);
	report_extension("texture compression dxt1",
		features[CEGL_FEATURE_TEXTURE_COMPRESSION_DXT1]);
	report_extension("texture lod",
		features[CEGL_FEATURE_TEXTURE_LOD]);
}

bool cegl_report_errors(void)
{
	bool reported = false;
	GLenum error;
	while (GL_NO_ERROR != (error = glGetError())) {
		logging_error("OpenGL error %u: %s.", error, gluErrorString(error));
		reported = true;
	}
	return reported;
}

bool cegl_query_feature(cegl_feature feature)
{
	return features[feature];
}

int cegl_max_texture_size(void)
{
	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	return max_texture_size;
}
