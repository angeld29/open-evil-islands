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

/*
 *  Based on:
 *  1. MSDN website (Programming Guide for DDS, Reference for DDS).
 *  2. DDS GIMP plugin (C) 2004-2008 Shawn Kirst <skirst@insightbb.com>,
 *     with parts (C) 2003 Arne Reuter <homepage@arnereuter.de>.
 *  3. SOIL (Simple OpenGL Image Library) (C) Jonathan Dummer.
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "celib.h"
#include "cegl.h"
#include "cebyteorder.h"
#include "cealloc.h"
#include "cemath.h"
#include "logging.h"
#include "mmpfile.h"
#include "texture.h"

struct texture {
	GLuint id;
};

static void setup_mag_min_params(int mipmap_count)
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
			static bool reported;
			if (!reported) {
				logging_warning("Some OpenGL features are not available. "
								"Mipmapping was disabled.");
				reported = true;
			}
		}
#endif /* GL_VERSION_1_2 */
	}
}

static bool scale_texture(int* width, int* height,
		GLenum data_format, GLenum data_type, void* data)
{
	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

	int new_width = ce_min(*width, max_texture_size);
	int new_height = ce_min(*height, max_texture_size);

	if (!ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_NON_POWER_OF_TWO)) {
		float int_width, int_height;
		float fract_width = modff(log2f(new_width), &int_width);
		float fract_height = modff(log2f(new_height), &int_height);

		if (!cefiszero(fract_width, CEEPS_E4)) {
			new_width = powf(2.0f, int_width);
		}
		if (!cefiszero(fract_height, CEEPS_E4)) {
			new_height = powf(2.0f, int_height);
		}
	}

	if (*width != new_width || *height != new_height) {
		int error = gluScaleImage(data_format, *width, *height,
			data_type, data, new_width, new_height, data_type, data);
		if (GL_NO_ERROR != error) {
			logging_error("gluScaleImage failed: %d (%s).",
				error, gluErrorString(error));
			return false;
		}

		*width = new_width;
		*height = new_height;
	}

	return true;
}

static bool specify_texture(int level, GLenum internal_format, int width,
		int height, GLenum data_format, GLenum data_type, void* data)
{
	if (!scale_texture(&width, &height, data_format, data_type, data)) {
		logging_error("Could not scale texture.");
		return false;
	}

	GLint alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

	const bool not_aligned = 0 != width % alignment;

	if (not_aligned) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	glTexImage2D(GL_TEXTURE_2D, level, internal_format,
		width, height, 0, data_format, data_type, data);

	if (not_aligned) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	}

	if (ce_gl_report_errors()) {
		logging_error("glTexImage2D failed.");
		return false;
	}

	return true;
}

static bool generate_texture(int mipmap_count, GLenum internal_format, int width,
		int height, int bpp, GLenum data_format, GLenum data_type, void* data)
{
	setup_mag_min_params(mipmap_count);

	uint8_t* src = data;

	for (int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		if (!specify_texture(i, internal_format, width,
				height, data_format, data_type, src)) {
			logging_error("Could not specify texture.");
			return false;
		}
		src += width * height * bpp;
	}

	return true;
}

static int dxt_blerp(int a, int b, int x)
{
	int t = (b - a) * x + 128;
	return a + ((t + (t >> 8)) >> 8);
}

static void dxt_lerp_rgb(uint8_t* dst, uint8_t* a, uint8_t* b, int f)
{
	dst[0] = dxt_blerp(a[0], b[0], f);
	dst[1] = dxt_blerp(a[1], b[1], f);
	dst[2] = dxt_blerp(a[2], b[2], f);
}

static void dxt_rgb565_to_bgr(uint8_t* dst, uint16_t v)
{
	uint8_t r = (v >> 11) & 0x1f;
	uint8_t g = (v >> 5) & 0x3f;
	uint8_t b = v & 0x1f;

	dst[0] = (b << 3) | (b >> 2);
	dst[1] = (g << 2) | (g >> 4);
	dst[2] = (r << 3) | (r >> 2);
}

static void dxt_decode_color_block(uint8_t* dst, uint8_t* src,
										int rowbytes, int format)
{
	uint8_t colors[4][3];

	uint16_t c0 = (uint16_t)src[0] | ((uint16_t)src[1] << 8);
	uint16_t c1 = (uint16_t)src[2] | ((uint16_t)src[3] << 8);

	dxt_rgb565_to_bgr(colors[0], c0);
	dxt_rgb565_to_bgr(colors[1], c1);

	if (c0 > c1) {
		dxt_lerp_rgb(colors[2], colors[0], colors[1], 0x55);
		dxt_lerp_rgb(colors[3], colors[0], colors[1], 0xaa);
	} else {
		for (int i = 0; i < 3; ++i) {
			colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
			colors[3][i] = 255;
		}
	}

	src += 4;

	for (int y = 0; y < 4; ++y) {
		uint8_t* d = dst + (y * rowbytes);
		uint32_t indexes = src[y];
		for (int x = 0; x < 4; ++x, d += 4, indexes >>= 2) {
			uint32_t idx = indexes & 0x03;
			d[0] = colors[idx][2];
			d[1] = colors[idx][1];
			d[2] = colors[idx][0];
			if (MMP_DXT1 == format) {
				d[3] = (c0 <= c1 && 3 == idx) ? 0 : 255;
			}
		}
	}
}

static void dxt_decode_alpha_block(uint8_t* dst, uint8_t* src, int rowbytes)
{
	for (int y = 0; y < 4; ++y) {
		uint8_t* d = dst + (y * rowbytes);
		uint16_t bits = (uint16_t)src[2 * y] | ((uint16_t)src[2 * y + 1] << 8);
		for (int x = 0; x < 4; ++x, d += 4, bits >>= 4) {
			d[0] = (bits & 0x0f) * 17;
		}
	}
}

static void dxt_decompress(uint8_t* dst, uint8_t* src,
							int width, int height, int format)
{
	assert(0 == (width & 3) && 0 == (height & 3));
	assert(width >= 4 && height >= 4);

	uint8_t* s = src;
	int rowbytes = width * 4;

	for (int y = 0; y < height; y += 4) {
		for (int x = 0; x < width; x += 4) {
			uint8_t* d = dst + (y * width + x) * 4;
			if (MMP_DXT3 == format) {
				dxt_decode_alpha_block(d + 3, s, rowbytes);
				s += 8;
			}
			dxt_decode_color_block(d, s, rowbytes, format);
			s += 8;
		}
	}
}

#ifdef GL_VERSION_1_3
static bool dxt_generate_texture_directly(int mipmap_count,
		int width, int height, int format, const void* data)
{
	setup_mag_min_params(mipmap_count);

	const uint8_t* src = data;

	for (int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		int data_size = ((width + 3) >> 2) *
						((height + 3) >> 2) * (MMP_DXT1 == format ? 8 : 16);

		glCompressedTexImage2D(GL_TEXTURE_2D, i, MMP_DXT1 == format ?
			CE_GL_COMPRESSED_RGB_S3TC_DXT1 : CE_GL_COMPRESSED_RGBA_S3TC_DXT3,
			width, height, 0, data_size, src);

		if (ce_gl_report_errors()) {
			logging_error("glCompressedTexImage2D failed.");
			return false;
		}

		src += data_size;
	}

	return true;
}
#endif /* GL_VERSION_1_3 */

static bool dxt_generate_texture(int mipmap_count,
		int width, int height, int format, void* data)
{
#ifdef GL_VERSION_1_3
	if ((ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_COMPRESSION_S3TC) ||
			(MMP_DXT1 == format &&
				ce_gl_query_feature(CE_GL_FEATURE_TEXTURE_COMPRESSION_DXT1))) &&
			dxt_generate_texture_directly(mipmap_count, width,
											height, format, data)) {
		return true;
	}
#endif /* GL_VERSION_1_3 */

	uint8_t* src = data;

	int data_size = 0;
	for (int i = 0, w = width, h = height;
			i < mipmap_count; ++i, w >>= 1, h >>= 1) {
		data_size += w * h * 4;
	}

	if (NULL == (data = ce_alloc(data_size))) {
		logging_error("Could not allocate memory.");
		return false;
	}

	uint8_t* dst = data;

	for (int i = 0, w = width, h = height;
			i < mipmap_count; ++i, w >>= 1, h >>= 1) {
		dxt_decompress(dst, src, w, h, format);
		src += ((w + 3) >> 2) * ((h + 3) >> 2) * (MMP_DXT1 == format ? 8 : 16);
		dst += w * h * 4;
	}

	bool ok = generate_texture(mipmap_count, GL_RGBA, width, height,
		4, GL_RGBA, GL_UNSIGNED_BYTE, data);

	ce_free(data, data_size);
	return ok;
}

#ifndef GL_VERSION_1_2
static bool generic16_argb_generate_texture_packed(int mipmap_count, int width,
											int height, int format, void* data)
{
	uint16_t* src = data;
	GLenum data_type;
	int ashift, rgbshift;

	switch (format) {
	case MMP_A1RGB5:
		data_type = CE_GL_UNSIGNED_SHORT_5_5_5_1;
		rgbshift = 1; ashift = 15;
		break;
	case MMP_ARGB4:
		data_type = CE_GL_UNSIGNED_SHORT_4_4_4_4;
		rgbshift = 4; ashift = 12;
		break;
	default:
		assert(false);
	}

	for (int i = 0, w = width, h = height;
			i < mipmap_count; ++i, w >>= 1, h >>= 1) {
		for (uint16_t *end = src + w * h; src != end; ++src) {
			*src = *src << rgbshift | *src >> ashift;
		}
	}

	return generate_texture(mipmap_count, GL_RGBA,
		width, height, 2, GL_RGBA, data_type, data);
}

static bool generic16_generate_texture(int mipmap_count, int width,
										int height, int format, void* data)
{
	GLenum internal_and_data_format;
	int bpp, rshift, gshift, ashift, rdiv, gdiv, bdiv, adiv;
	uint16_t rmask, gmask, bmask;

	switch (format) {
	case MMP_R5G6B5:
		internal_and_data_format = GL_RGB;
		bpp = 3;
		rshift = 11; gshift = 5;
		rdiv = 31; gdiv = 63; bdiv = 31;
		rmask = 0xf800; gmask = 0x7e0; bmask = 0x1f;
		break;
	case MMP_A1RGB5:
		internal_and_data_format = GL_RGBA;
		bpp = 4;
		rshift = 10; gshift = 5; ashift = 15;
		rdiv = 31; gdiv = 31; bdiv = 31; adiv = 1;
		rmask = 0x7c00; gmask = 0x3e0; bmask = 0x1f;
		break;
	case MMP_ARGB4:
		internal_and_data_format = GL_RGBA;
		bpp = 4;
		rshift = 8; gshift = 4; ashift = 12;
		rdiv = 15; gdiv = 15; bdiv = 15; adiv = 15;
		rmask = 0xf00; gmask = 0xf0; bmask = 0xf;
		break;
	default:
		assert(false);
	}

	if (4 == bpp && ce_gl_query_feature(CE_GL_FEATURE_PACKED_PIXELS)) {
		return generic16_argb_generate_texture_packed(mipmap_count, width,
														height, format, data);
	}

	int pixel_count = 0;
	for (int i = 0, w = width, h = height;
			i < mipmap_count; ++i, w >>= 1, h >>= 1) {
		pixel_count += w * h;
	}

	uint16_t* src = data;
	uint16_t* end = src + pixel_count;
	int data_size = bpp * pixel_count;

	if (NULL == (data = ce_alloc(data_size))) {
		return false;
	}

	for (uint8_t* dst = data; src != end; ++src) {
		*dst++ = ((*src & rmask) >> rshift) * 255 / rdiv;
		*dst++ = ((*src & gmask) >> gshift) * 255 / gdiv;
		*dst++ = (*src & bmask) * 255 / bdiv;
		if (4 == bpp) {
			*dst++ = (*src >> ashift) * 255 / adiv;
		}
	}

	bool ok = generate_texture(mipmap_count, internal_and_data_format,
		width, height, bpp, internal_and_data_format, GL_UNSIGNED_BYTE, data);

	ce_free(data, data_size);
	return ok;
}

static bool argb8_generate_texture(int mipmap_count, int width,
									int height, void* data)
{
	static uint8_t dst[4];
	uint32_t* src = data;

	for (int i = 0, w = width, h = height;
			i < mipmap_count; ++i, w >>= 1, h >>= 1) {
		for (uint32_t *end = src + w * h; src != end; ++src) {
			if (ce_gl_query_feature(CE_GL_FEATURE_PACKED_PIXELS)) {
				*src = *src << 8 | *src >> 24;
			} else {
				dst[0] = (*src & 0xff0000) >> 16;
				dst[1] = (*src & 0xff00) >> 8;
				dst[2] = *src & 0xff;
				dst[3] = *src >> 24;
				*src = *(uint32_t*)dst;
			}
		}
	}

	return generate_texture(mipmap_count, GL_RGBA, width, height,
		4, GL_RGBA, ce_gl_query_feature(CE_GL_FEATURE_PACKED_PIXELS) ?
		CE_GL_UNSIGNED_INT_8_8_8_8 : GL_UNSIGNED_BYTE, data);
}
#endif /* !GL_VERSION_1_2 */

static bool pnt3_generate_texture(int size, int width, int height, void* data)
{
	int data_size = width * height * 4;

	if (size < data_size) { // PNT3 compressed
		uint32_t* src = data;
		uint32_t* end = src + size / sizeof(uint32_t);

		if (NULL == (data = ce_alloc(data_size))) {
			logging_error("Could not allocate memory.");
			return false;
		}

		uint8_t* dst = data;
		int n = 0;

		while (src != end) {
			uint32_t value = ce_le2cpu32(*src++);
			if (0 < value && value < 1000000) {
				memcpy(dst, src - 1 - n, n * sizeof(uint32_t));
				dst += n * sizeof(uint32_t);
				n = 0;
				memset(dst, '\0', value);
				dst += value;
			} else {
				++n;
			}
		}
		memcpy(dst, src - n, n * sizeof(uint32_t));
	}

#ifdef GL_VERSION_1_2
	bool ok = generate_texture(1, GL_RGBA, width, height,
		4, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
#else
	bool ok = argb8_generate_texture(1, width, height, data);
#endif

	if (size < data_size) {
		ce_free(data, data_size);
	}

	return ok;
}

texture* texture_open(void* data)
{
	texture* tex = ce_alloc(sizeof(texture));
	if (NULL == tex) {
		return NULL;
	}

	glGenTextures(1, &tex->id);

	glBindTexture(GL_TEXTURE_2D, tex->id);

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

	uint32_t* mmp = data;

	if (MMP_SIGNATURE != ce_le2cpu32(*mmp++)) {
		logging_error("Wrong mmp signature.");
		texture_close(tex);
		return NULL;
	}

	uint32_t width = ce_le2cpu32(*mmp++);
	uint32_t height = ce_le2cpu32(*mmp++);
	uint32_t mipmap_count_or_size = ce_le2cpu32(*mmp++);
	uint32_t format = ce_le2cpu32(*mmp++);

	mmp += 14;

	bool ok;

	switch (format) {
	case MMP_DXT1:
	case MMP_DXT3:
		ok = dxt_generate_texture(mipmap_count_or_size,
									width, height, format, mmp);
		break;
	case MMP_PNT3:
		ok = pnt3_generate_texture(mipmap_count_or_size, width, height, mmp);
		break;
	case MMP_R5G6B5:
#ifdef GL_VERSION_1_2
		ok = generate_texture(mipmap_count_or_size, GL_RGB,
			width, height, 2, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, mmp);
#else
		ok = generic16_generate_texture(mipmap_count_or_size,
										width, height, format, mmp);
#endif
		break;
	case MMP_A1RGB5:
#ifdef GL_VERSION_1_2
		ok = generate_texture(mipmap_count_or_size, GL_RGBA,
			width, height, 2, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, mmp);
#else
		ok = generic16_generate_texture(mipmap_count_or_size,
										width, height, format, mmp);
#endif
		break;
	case MMP_ARGB4:
#ifdef GL_VERSION_1_2
		ok = generate_texture(mipmap_count_or_size, GL_RGBA,
			width, height, 2, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, mmp);
#else
		ok = generic16_generate_texture(mipmap_count_or_size,
										width, height, format, mmp);
#endif
		break;
	case MMP_ARGB8:
#ifdef GL_VERSION_1_2
		ok = generate_texture(mipmap_count_or_size, GL_RGBA,
			width, height, 4, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, mmp);
#else
		ok = argb8_generate_texture(mipmap_count_or_size, width, height, mmp);
#endif
		break;
	default:
		assert(false);
		ok = false;
	}

	if (!ok) {
		texture_close(tex);
		return NULL;
	}

	return tex;
}

void texture_close(texture* tex)
{
	if (NULL == tex) {
		return;
	}

	glDeleteTextures(1, &tex->id);

	ce_free(tex, sizeof(texture));
}

void texture_bind(texture* tex)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->id);
}

void texture_unbind(texture* tex)
{
	ce_unused(tex);
	glDisable(GL_TEXTURE_2D);
}
