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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "cegl.h"
#include "celib.h"
#include "cemath.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cestring.h"
#include "cetexture.h"

struct ce_texture {
	ce_string* name;
	int ref_count;
	GLuint id;
};

static void ce_texture_setup_filters(int mipmap_count)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mipmap_count > 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
										GL_LINEAR_MIPMAP_LINEAR);
#ifdef GL_VERSION_1_2
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
#else
		if (ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_LOD)) {
			glTexParameteri(GL_TEXTURE_2D, CE_GL_TEXTURE_MAX_LEVEL,
												mipmap_count - 1);
		} else if (ce_gl_query_feature(CE_GL_FEATURE_GENERATE_MIPMAP)) {
			glTexParameteri(GL_TEXTURE_2D, CE_GL_GENERATE_MIPMAP, GL_TRUE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			// ok, all opengl's code is NOT thread safe
			static bool reported;
			if (!reported) {
				ce_logging_warning("texture: some opengl features are not "
									"available, mipmapping was disabled");
				reported = true;
			}
		}
#endif
	}
}

static void ce_texture_specify(int width, int height, int level,
	GLenum internal_format, GLenum data_format, GLenum data_type, void* data)
{
	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

	int new_width = ce_min(width, max_texture_size);
	int new_height = ce_min(height, max_texture_size);

	if (!ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO)) {
		float int_width, int_height;
		float fract_width = modff(log2f(new_width), &int_width);
		float fract_height = modff(log2f(new_height), &int_height);

		if (!ce_fiszero(fract_width, CE_EPS_E3)) {
			new_width = powf(2.0f, int_width);
		}
		if (!ce_fiszero(fract_height, CE_EPS_E3)) {
			new_height = powf(2.0f, int_height);
		}
	}

	if (width != new_width || height != new_height) {
		gluScaleImage(data_format, width, height,
			data_type, data, new_width, new_height, data_type, data);
		width = new_width;
		height = new_height;
	}

	glTexImage2D(GL_TEXTURE_2D, level, internal_format,
		width, height, 0, data_format, data_type, data);
}

static void ce_texture_generate(ce_mmpfile* mmpfile,
	GLenum internal_format, GLenum data_format, GLenum data_type)
{
	ce_texture_setup_filters(mmpfile->mipmap_count);

	// most ei's textures of width divisible by 4 (gl's default row alignment)
	const bool not_aligned = 0 != mmpfile->width % 4;

	if (not_aligned) {
		glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	uint8_t* src = mmpfile->texels;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		ce_texture_specify(width, height, i,
			internal_format, data_format, data_type, src);
		src += ce_mmpfile_storage_size(width, height, 1, mmpfile->bit_count);
	}

	if (not_aligned) {
		glPopClientAttrib();
	}
}

static void ce_texture_generate_compressed(ce_mmpfile* mmpfile,
											GLenum internal_format)
{
	ce_texture_setup_filters(mmpfile->mipmap_count);

	const uint8_t* src = mmpfile->texels;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		int size = ce_mmpfile_storage_size(width, height, 1, mmpfile->bit_count);
		ce_gl_compressed_tex_image_2d(GL_TEXTURE_2D,
			i, internal_format, width, height, 0, size, src);
		src += size;
	}
}

static void ce_texture_generate_dxt(ce_mmpfile* mmpfile)
{
	if (ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC) ||
			(CE_MMPFILE_FORMAT_DXT1 == mmpfile->format &&
			ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1))) {
		ce_texture_generate_compressed(mmpfile, (GLenum[])
			{ CE_GL_COMPRESSED_RGBA_S3TC_DXT1, CE_GL_COMPRESSED_RGBA_S3TC_DXT3 }
			[CE_MMPFILE_FORMAT_DXT3 == mmpfile->format]);
	} else {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	}
}

static void ce_texture_generate_r5g6b5(ce_mmpfile* mmpfile)
{
#ifdef GL_VERSION_1_2
	ce_texture_generate(mmpfile, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
#else
	ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
	ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
#endif
}

static void ce_texture_generate_a1rgb5(ce_mmpfile* mmpfile)
{
#ifdef GL_VERSION_1_2
	ce_texture_generate(mmpfile, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV);
#else
	if (ce_gl_query_feature(CE_GL_FEATURE_PACKED_PIXELS)) {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_RGB5A1);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, CE_GL_UNSIGNED_SHORT_5_5_5_1);
	} else {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	}
#endif
}

static void ce_texture_generate_argb4(ce_mmpfile* mmpfile)
{
#ifdef GL_VERSION_1_2
	ce_texture_generate(mmpfile, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV);
#else
	if (ce_gl_query_feature(CE_GL_FEATURE_PACKED_PIXELS)) {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_RGBA4);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, CE_GL_UNSIGNED_SHORT_4_4_4_4);
	} else {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	}
#endif
}

static void ce_texture_generate_argb8(ce_mmpfile* mmpfile)
{
#ifdef GL_VERSION_1_2
	ce_texture_generate(mmpfile, GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV);
#else
	if (ce_gl_query_feature(CE_GL_FEATURE_PACKED_PIXELS)) {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_RGBA8);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, CE_GL_UNSIGNED_INT_8_8_8_8);
	} else {
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
		ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	}
#endif
}

ce_texture* ce_texture_new(const char* name, ce_mmpfile* mmpfile)
{
	ce_texture* texture = ce_alloc(sizeof(ce_texture));
	texture->name = ce_string_new_str(name);
	texture->ref_count = 1;

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	switch (mmpfile->format) {
	case CE_MMPFILE_FORMAT_DXT1:
	case CE_MMPFILE_FORMAT_DXT3:
		ce_texture_generate_dxt(mmpfile);
		break;
	case CE_MMPFILE_FORMAT_R5G6B5:
		ce_texture_generate_r5g6b5(mmpfile);
		break;
	case CE_MMPFILE_FORMAT_A1RGB5:
		ce_texture_generate_a1rgb5(mmpfile);
		break;
	case CE_MMPFILE_FORMAT_ARGB4:
		ce_texture_generate_argb4(mmpfile);
		break;
	case CE_MMPFILE_FORMAT_PNT3:
	case CE_MMPFILE_FORMAT_ARGB8:
		ce_texture_generate_argb8(mmpfile);
		break;
	default:
		assert(false);
	}

	if (ce_gl_report_errors()) {
		ce_logging_error("texture: opengl failed");
	}

	return texture;
}

void ce_texture_del(ce_texture* texture)
{
	if (NULL != texture) {
		assert(texture->ref_count > 0);
		if (0 == --texture->ref_count) {
			glDeleteTextures(1, &texture->id);
			ce_string_del(texture->name);
			ce_free(texture, sizeof(ce_texture));
		}
	}
}

void ce_texture_wrap(ce_texture* texture, ce_texture_wrap_mode mode)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);

	if (CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE == mode) {
#ifdef GL_VERSION_1_2
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
		if (ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_EDGE_CLAMP)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CE_GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CE_GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
#endif
	} else if (CE_TEXTURE_WRAP_MODE_CLAMP == mode) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

bool ce_texture_equal(const ce_texture* texture, const ce_texture* other)
{
	return texture->id == other->id;
}

const char* ce_texture_get_name(ce_texture* texture)
{
	return texture->name->str;
}

void ce_texture_bind(ce_texture* texture)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture->id);
}

void ce_texture_unbind(ce_texture* texture)
{
	ce_unused(texture);
	glDisable(GL_TEXTURE_2D);
}

ce_texture* ce_texture_add_ref(ce_texture* texture)
{
	++texture->ref_count;
	return texture;
}
