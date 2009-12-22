/*  Based on:
 *  1. MSDN website (Programming Guide for DDS, Reference for DDS).
 *  2. DDS GIMP plugin (C) 2004-2008 Shawn Kirst <skirst@insightbb.com>,
 *     with parts (C) 2003 Arne Reuter <homepage@arnereuter.de>.
 *  3. SOIL (Simple OpenGL Image Library) (C) Jonathan Dummer.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "gllib.h"
#include "byteorder.h"
#include "memfile.h"
#include "mmpfile.h"

/*  MPP file format:
 *
 *  width: uint32 little-endian
 *  height: uint32 little-endian
 *  size (PNT3) or mipmap count (other): uint32 little-endian
 *  format: uint32 little-endian
 *  rgb_bit_count: uint32 little-endian
 *  a_bit_mask: uint32 little-endian
 *  unknown: uint32 little-endian
 *  unknown: uint32 little-endian
 *  r_bit_mask: uint32 little-endian
 *  unknown: uint32 little-endian
 *  unknown: uint32 little-endian
 *  g_bit_mask: uint32 little-endian
 *  unknown: uint32 little-endian
 *  unknown: uint32 little-endian
 *  b_bit_mask: uint32 little-endian
 *  unknown: uint32 little-endian
 *  unknown: uint32 little-endian
 *  unknown: uint32 little-endian
*/

enum {
	MMP_SIGNATURE = 0x504d4d
};

enum {
	MMP_DD = 0x4444,
	MMP_DXT1 = 0x31545844,
	MMP_DXT3 = 0x33545844,
	MMP_PNT3 = 0x33544e50,
	MMP_PV = 0x5650,
	MMP_QU = 0x5551,
	MMP_XX = 0x8888
};

static bool raw_generate_texture(GLenum internal_format,
		GLenum data_format, GLenum data_type, int width,
		int height, int mipmap_count, int bpp, memfile* mem)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mipmap_count > 1) {
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
	}

	size_t data_size = width * height * bpp;

	void* data = malloc(data_size);
	if (NULL == data || 1 != memfile_read(data, data_size, 1, mem)) {
		free(data);
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
		width, height, 0, data_format, data_type, data);

	for (int i = 1; i < mipmap_count; ++i) {
		int mipmap_width = width >> i;
		int mipmap_height = height >> i;

		if (1 != memfile_read(data, mipmap_width * mipmap_height * bpp, 1, mem)) {
			free(data);
			return false;
		}

		glTexImage2D(GL_TEXTURE_2D, i, internal_format,
			mipmap_width, mipmap_height, 0, data_format, data_type, data);
	}

	free(data);
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
								int w, int h, int rowbytes, int format)
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

	for (int y = 0; y < h; ++y) {
		uint8_t* d = dst + (y * rowbytes);
		uint32_t indexes = src[y];
		for (int x = 0; x < w; ++x, d += 4, indexes >>= 2) {
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

static void dxt_decode_alpha_block(uint8_t* dst, uint8_t* src,
								int w, int h, int rowbytes)
{
	for (int y = 0; y < h; ++y) {
		uint8_t* d = dst + (y * rowbytes);
		uint16_t bits = (uint16_t)src[2 * y] | ((uint16_t)src[2 * y + 1] << 8);
		for (int x = 0; x < w; ++x, d += 4, bits >>= 4) {
			d[0] = (bits & 0x0f) * 17;
		}
	}
}

static bool dxt_decompress(uint8_t* dst, uint8_t* src,
							int width, int height, int format)
{
	if (0 != (width & 3) || 0 != (height & 3)) {
		return false;
	}

	int sx = (width < 4) ? width : 4;
	int sy = (height < 4) ? height : 4;

	uint8_t* s = src;

	for (int y = 0; y < height; y += 4) {
		for (int x = 0; x < width; x += 4) {
			uint8_t* d = dst + (y * width + x) * 4;
			if (MMP_DXT3 == format) {
				dxt_decode_alpha_block(d + 3, s, sx, sy, width * 4);
				s += 8;
			}
			dxt_decode_color_block(d, s, sx, sy, width * 4, format);
			s += 8;
		}
	}

	return true;
}

static bool dxt_generate_texture_indirectly(int width, int height,
						int mipmap_count, int format, memfile* mem)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mipmap_count > 1) {
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
	}

	int buffer_size = ((width + 3) >> 2) * ((height + 3) >> 2);
	buffer_size *= (MMP_DXT1 == format) ? 8 : 16;

	void* buffer = malloc(buffer_size);
	if (NULL == buffer || 1 != memfile_read(buffer, buffer_size, 1, mem)) {
		free(buffer);
		return false;
	}

	void* data = malloc(width * height * 4);
	if (NULL == data || !dxt_decompress(data, buffer,
							width, height, format)) {
		free(buffer);
		free(data);
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
		height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	for (int i = 1; i < mipmap_count; ++i) {
		int mipmap_width = width >> i;
		int mipmap_height = height >> i;

		int mipmap_buffer_size = ((mipmap_width + 3) >> 2) *
									((mipmap_height + 3) >> 2);
		mipmap_buffer_size *= (MMP_DXT1 == format) ? 8 : 16;

		if (1 != memfile_read(buffer, mipmap_buffer_size, 1, mem) ||
				!dxt_decompress(data, buffer, mipmap_width,
									mipmap_height, format)) {
			free(buffer);
			free(data);
			return false;
		}

		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, mipmap_width,
			mipmap_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	free(buffer);
	free(data);
	return true;
}

static bool dxt_generate_texture_directly(int width, int height,
					int mipmap_count, int format, memfile* mem)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mipmap_count > 1) {
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
	}

	int data_size = ((width + 3) >> 2) * ((height + 3) >> 2);
	data_size *= (MMP_DXT1 == format) ? 8 : 16;

	void* data = malloc(data_size);
	if (NULL == data || 1 != memfile_read(data, data_size, 1, mem)) {
		free(data);
		return false;
	}

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, (MMP_DXT1 == format) ?
		GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
		width, height, 0, data_size, data);

	for (int i = 1; i < mipmap_count; ++i) {
		int mipmap_width = width >> i;
		int mipmap_height = height >> i;

		int mipmap_data_size = ((mipmap_width + 3) >> 2) *
									((mipmap_height + 3) >> 2);
		mipmap_data_size *= (MMP_DXT1 == format) ? 8 : 16;

		if (1 != memfile_read(data, mipmap_data_size, 1, mem)) {
			free(data);
			return false;
		}

		glCompressedTexImage2D(GL_TEXTURE_2D, i, (MMP_DXT1 == format) ?
			GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
			mipmap_width, mipmap_height, 0, mipmap_data_size, data);
	}

	free(data);
	return true;
}

static bool pnt3_generate_texture(int width, int height, int size, memfile* mem)
{
	uint8_t* data;
	int data_size = width * height * 4;

	if (size < data_size) { // PNT3 compressed
		data = malloc(data_size);
		if (NULL == data) {
			return false;
		}

		for (uint8_t* d = data; ;) {
			uint32_t le_value;
			if (0 == memfile_read(&le_value, sizeof(uint32_t), 1, mem)) {
				break;
			}
			uint32_t cpu_value = le2cpu32(le_value);
			if (0 < cpu_value && cpu_value < 1000000) {
				memset(d, '\0', cpu_value);
				d += cpu_value;
			} else {
				memcpy(d, &le_value, sizeof(uint32_t));
				d += sizeof(uint32_t);
			}
		}

		mem = memfile_open_data(data, data_size, "rb");
		if (NULL == mem) {
			free(data);
			return false;
		}
	}

	bool ok = raw_generate_texture(GL_RGBA, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, width, height, 0, 4, mem);

	if (size < data_size) {
		memfile_close(mem);
		free(data);
	}

	return ok;
}

GLuint mmpfile_generate_texture(GLuint id, memfile* mem)
{
	// TODO: possible leak

	if (0 == id) {
		glGenTextures(1, &id);
	}

	if (0 == id) {
		return 0;
	}

	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		return 0;
	}

	le2cpu32s(&signature);
	if (MMP_SIGNATURE != signature) {
		return 0;
	}

	uint32_t width;
	uint32_t height;
	uint32_t mipmap_count_or_size;
	uint32_t format;
	uint32_t unused_or_unknown[14];

	if (1 != memfile_read(&width, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&height, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mipmap_count_or_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&format, sizeof(uint32_t), 1, mem) ||
			14 != memfile_read(unused_or_unknown, sizeof(uint32_t), 14, mem)) {
		return 0;
	}

	le2cpu32s(&width);
	le2cpu32s(&height);
	le2cpu32s(&mipmap_count_or_size);
	le2cpu32s(&format);

	bool ok;
	switch (format) {
	case MMP_DXT1:
	case MMP_DXT3:
		ok = gl_query_feature(GL_FEATURE_TEXTURE_COMPRESSION_S3TC) ?
			dxt_generate_texture_directly(width, height,
				mipmap_count_or_size, format, mem) :
			dxt_generate_texture_indirectly(width, height,
				mipmap_count_or_size, format, mem);
		break;
	case MMP_DD:
		ok = raw_generate_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV,
			width, height, mipmap_count_or_size, 2, mem);
		break;
	case MMP_PNT3:
		ok = pnt3_generate_texture(width, height, mipmap_count_or_size, mem);
		break;
	case MMP_PV:
		ok = raw_generate_texture(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
			width, height, mipmap_count_or_size, 2, mem);
		break;
	case MMP_QU:
		ok = raw_generate_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
			width, height, mipmap_count_or_size, 2, mem);
		break;
	case MMP_XX:
		ok = raw_generate_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
			width, height, mipmap_count_or_size, 4, mem);
		break;
	default:
		assert(false);
		ok = false;
	}

	if (!ok) {
		return 0;
	}

	return id;
}
