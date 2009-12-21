/* Based on:
   1. MSDN website (Programming Guide for DDS, Reference for DDS).
   2. DDS GIMP plugin (C) 2004-2008 Shawn Kirst <skirst@insightbb.com>,
      with parts (C) 2003 Arne Reuter <homepage@arnereuter.de>.
   3. SOIL (Simple OpenGL Image Library) (C) Jonathan Dummer. */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "byteorder.h"
#include "memfile.h"
#include "mmpfile.h"

/* MPP file format:
   width: uint32 little-endian
   height: uint32 little-endian
   size (PNT3) or mipmap count (other): uint32 little-endian
   format: uint32 little-endian
   rgb_bit_count: uint32 little-endian
   a_bit_mask: uint32 little-endian
   unknown: uint32 little-endian
   unknown: uint32 little-endian
   r_bit_mask: uint32 little-endian
   unknown: uint32 little-endian
   unknown: uint32 little-endian
   g_bit_mask: uint32 little-endian
   unknown: uint32 little-endian
   unknown: uint32 little-endian
   b_bit_mask: uint32 little-endian
   unknown: uint32 little-endian
   unknown: uint32 little-endian
   unknown: uint32 little-endian */

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

// TODO: remove mmpfile
typedef struct mmpfile {
	uint32_t width;
	uint32_t height;
	uint32_t mipmap_count_or_size;
	uint32_t format;
	uint32_t rgb_bit_count;
	uint32_t a_bit_mask;
	uint32_t r_bit_mask;
	uint32_t g_bit_mask;
	uint32_t b_bit_mask;
} mmpfile;

static bool gl_extension_supported(const char* extension)
{
	return NULL != strstr((const char*)glGetString(GL_EXTENSIONS), extension);
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

static bool dxt_create_texture_indirectly(int width, int height,
						int mipmap_count, int format, memfile* mem)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mipmap_count > 1) {
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
	}

	size_t buffer_size = ((width + 3) >> 2) * ((height + 3) >> 2);
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
		return false;;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
		height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	for (int i = 1; i < mipmap_count; ++i) {
		int mipmap_width = width >> i;
		int mipmap_height = height >> i;

		size_t mipmap_buffer_size = ((mipmap_width + 3) >> 2) *
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

static bool dxt_create_texture_directly(int width, int height,
					int mipmap_count, int format, memfile* mem)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mipmap_count > 1) {
		glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
	}

	size_t data_size = ((width + 3) >> 2) * ((height + 3) >> 2);
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

		size_t mipmap_data_size = ((mipmap_width + 3) >> 2) *
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

static bool raw_create_texture(GLenum internal_format,
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

	// TODO: check capability POWER_OF_TWO and tex max supported size

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

static bool dd_create_texture(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0xf00 == mmp->r_bit_mask);
	assert(0xf0 == mmp->g_bit_mask);
	assert(0xf == mmp->b_bit_mask);
	assert(0xf000 == mmp->a_bit_mask);
	return raw_create_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV,
		mmp->width, mmp->height, mmp->mipmap_count_or_size, 2, mem);
}

static bool pnt3_create_texture(mmpfile* mmp, memfile* mem)
{
	assert(4 == (mmp->rgb_bit_count >> 3));
	assert(0 == mmp->r_bit_mask);
	assert(0 == mmp->g_bit_mask);
	assert(0 == mmp->b_bit_mask);
	assert(0 == mmp->a_bit_mask);

	if (mmp->mipmap_count_or_size < mmp->width * mmp->height * 4) {
		uint8_t* data = malloc(mmp->width * mmp->height * 4);

		for (uint8_t* d = data; ;) {
			uint32_t value;
			if (0 == memfile_read(&value, sizeof(uint32_t), 1, mem)) {
				break;
			}
			if (0 < le2cpu32(value) && le2cpu32(value) < 1000000) {
				memset(d, '\0', value);
				d += value;
			} else {
				memcpy(d, &value, sizeof(uint32_t));
				d += sizeof(uint32_t);
			}
		}

		memfile* mem = memfile_open_data(data, mmp->width * mmp->height * 4, "rb");
		if (NULL == mem) {
			free(data);
			return false;
		}

		bool ok = raw_create_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
			mmp->width, mmp->height, 0, 4, mem);

		memfile_close(mem);
		return ok;
	}

	return raw_create_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
		mmp->width, mmp->height, 0, 4, mem);
}

static bool pv_create_texture(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0xf800 == mmp->r_bit_mask);
	assert(0x7e0 == mmp->g_bit_mask);
	assert(0x1f == mmp->b_bit_mask);
	assert(0 == mmp->a_bit_mask);
	return raw_create_texture(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
		mmp->width, mmp->height, mmp->mipmap_count_or_size, 2, mem);
}

static bool qu_create_texture(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0x7c00 == mmp->r_bit_mask);
	assert(0x3e0 == mmp->g_bit_mask);
	assert(0x1f == mmp->b_bit_mask);
	assert(0x8000 == mmp->a_bit_mask);
	return raw_create_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
		mmp->width, mmp->height, mmp->mipmap_count_or_size, 2, mem);
}

static bool xx_create_texture(mmpfile* mmp, memfile* mem)
{
	assert(4 == (mmp->rgb_bit_count >> 3));
	assert(0xff0000 == mmp->r_bit_mask);
	assert(0xff00 == mmp->g_bit_mask);
	assert(0xff == mmp->b_bit_mask);
	assert(0xff000000 == mmp->a_bit_mask);
	return raw_create_texture(GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
		mmp->width, mmp->height, mmp->mipmap_count_or_size, 4, mem);
}

GLuint mmpfile_create_texture(GLuint texid, memfile* mem)
{
	if (0 == texid) {
		glGenTextures(1, &texid);
	}

	if (0 == texid) {
		return 0;
	}

	glBindTexture(GL_TEXTURE_2D, texid);

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

	mmpfile mmp;
	uint32_t unknown;

	if (1 != memfile_read(&mmp.width, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.height, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.mipmap_count_or_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.format, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.rgb_bit_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.a_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.r_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.g_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp.b_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem)) {
		return 0;
	}

	le2cpu32s(&mmp.width);
	le2cpu32s(&mmp.height);
	le2cpu32s(&mmp.mipmap_count_or_size);
	le2cpu32s(&mmp.format);
	le2cpu32s(&mmp.rgb_bit_count);
	le2cpu32s(&mmp.a_bit_mask);
	le2cpu32s(&mmp.r_bit_mask);
	le2cpu32s(&mmp.g_bit_mask);
	le2cpu32s(&mmp.b_bit_mask);

	bool ok;
	switch (mmp.format) {
	case MMP_DXT1:
	case MMP_DXT3:
		ok = gl_extension_supported("GL_EXT_texture_compression_s3tc") ?
			dxt_create_texture_directly(mmp.width, mmp.height,
						mmp.mipmap_count_or_size, mmp.format, mem) :
			dxt_create_texture_indirectly(mmp.width, mmp.height,
						mmp.mipmap_count_or_size, mmp.format, mem);
		break;
	case MMP_DD:
		ok = dd_create_texture(&mmp, mem);
		break;
	case MMP_PNT3:
		ok = pnt3_create_texture(&mmp, mem);
		break;
	case MMP_PV:
		ok = pv_create_texture(&mmp, mem);
		break;
	case MMP_QU:
		ok = qu_create_texture(&mmp, mem);
		break;
	case MMP_XX:
		ok = xx_create_texture(&mmp, mem);
		break;
	default:
		assert(false);
		ok = false;
	}

	if (!ok) {
		return 0;
	}

	return texid;
}
