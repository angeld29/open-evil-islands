/* Based on:
   1. MSDN website (Programming Guide for DDS, Reference for DDS).
   2. DDS GIMP plugin (C) 2004-2008 Shawn Kirst <skirst@insightbb.com>,
      with parts (C) 2003 Arne Reuter <homepage@arnereuter.de>. */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "byteorder.h"
#include "memfile.h"
#include "mmpfile.h"

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

typedef struct mmpfile {
	uint32_t width;
	uint32_t height;
	uint32_t mipmap_count_or_size;
	uint32_t type;
	uint32_t rgb_bit_count;
	uint32_t a_bit_mask;
	uint32_t r_bit_mask;
	uint32_t g_bit_mask;
	uint32_t b_bit_mask;
	uint8_t* data;
	GLint internal_format;
	GLenum data_format;
	GLenum data_type;
} mmpfile;

static int blerp(int a, int b, int x)
{
	int t = (b - a) * x + 128;
	return a + ((t + (t >> 8)) >> 8);
}

static void lerp_rgb(uint8_t* dst, uint8_t* a, uint8_t* b, int f)
{
	dst[0] = blerp(a[0], b[0], f);
	dst[1] = blerp(a[1], b[1], f);
	dst[2] = blerp(a[2], b[2], f);
}

static void rgb565_to_bgr(uint8_t* dst, uint16_t v)
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

	rgb565_to_bgr(colors[0], c0);
	rgb565_to_bgr(colors[1], c1);

	if (c0 > c1) {
		lerp_rgb(colors[2], colors[0], colors[1], 0x55);
		lerp_rgb(colors[3], colors[0], colors[1], 0xaa);
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

static bool mmpfile_read_dxt(mmpfile* mmp, memfile* mem)
{
	mmp->data = malloc(mmp->width * mmp->height * 4);
	if (NULL == mmp->data) {
		return false;
	}

	size_t linear_size = ((mmp->width + 3) >> 2) * ((mmp->height + 3) >> 2);
	linear_size *= (MMP_DXT1 == mmp->type) ? 8 : 16;

	uint8_t* buffer = malloc(linear_size);
	if (NULL == buffer) {
		return false;
	}

	if (1 != memfile_read(buffer, linear_size, 1, mem)) {
		free(buffer);
		return false;
	}

	if (!dxt_decompress(mmp->data, buffer, mmp->width, mmp->height, mmp->type)) {
		free(buffer);
		return false;
	}

	mmp->internal_format = GL_RGBA;
	mmp->data_format = GL_RGBA;
	mmp->data_type = GL_UNSIGNED_BYTE;

	free(buffer);
	return true;
}

static bool mmpfile_read_raw(GLint internal_format, GLenum data_format,
					GLenum data_type, int bpp, mmpfile* mmp, memfile* mem)
{
	size_t data_size = mmp->width * mmp->height * bpp;

	mmp->data = malloc(data_size);
	if (NULL == mmp->data) {
		return false;
	}

	if (1 != memfile_read(mmp->data, data_size, 1, mem)) {
		return false;
	}

	mmp->internal_format = internal_format;
	mmp->data_format = data_format;
	mmp->data_type = data_type;

	return true;
}

static bool mmpfile_read_dd(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0xf00 == mmp->r_bit_mask);
	assert(0xf0 == mmp->g_bit_mask);
	assert(0xf == mmp->b_bit_mask);
	assert(0xf000 == mmp->a_bit_mask);
	return mmpfile_read_raw(GL_RGBA, GL_BGRA,
		GL_UNSIGNED_SHORT_4_4_4_4_REV, 2, mmp, mem);
}

static bool mmpfile_read_pnt3(mmpfile* mmp, memfile* mem)
{
	assert(4 == (mmp->rgb_bit_count >> 3));
	assert(0 == mmp->r_bit_mask);
	assert(0 == mmp->g_bit_mask);
	assert(0 == mmp->b_bit_mask);
	assert(0 == mmp->a_bit_mask);
	if (mmp->mipmap_count_or_size < 1024 * 256) {
		return false;
	}
	return mmpfile_read_raw(GL_RGBA, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, 4, mmp, mem);
}

static bool mmpfile_read_pv(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0xf800 == mmp->r_bit_mask);
	assert(0x7e0 == mmp->g_bit_mask);
	assert(0x1f == mmp->b_bit_mask);
	assert(0 == mmp->a_bit_mask);
	return mmpfile_read_raw(GL_RGB, GL_RGB,
		GL_UNSIGNED_SHORT_5_6_5, 2, mmp, mem);
}

static bool mmpfile_read_qu(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0x7c00 == mmp->r_bit_mask);
	assert(0x3e0 == mmp->g_bit_mask);
	assert(0x1f == mmp->b_bit_mask);
	assert(0x8000 == mmp->a_bit_mask);
	return mmpfile_read_raw(GL_RGBA, GL_BGRA,
		GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, mmp, mem);
}

static bool mmpfile_read_xx(mmpfile* mmp, memfile* mem)
{
	assert(4 == (mmp->rgb_bit_count >> 3));
	assert(0xff0000 == mmp->r_bit_mask);
	assert(0xff00 == mmp->g_bit_mask);
	assert(0xff == mmp->b_bit_mask);
	assert(0xff000000 == mmp->a_bit_mask);
	return mmpfile_read_raw(GL_RGBA, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, 4, mmp, mem);
}

static int mmpfile_close(mmpfile* mmp)
{
	if (NULL == mmp) {
		return 0;
	}

	free(mmp->data);

	free(mmp);

	return 0;
}

static mmpfile* mmpfile_open(memfile* mem)
{
	mmpfile* mmp = calloc(1, sizeof(mmpfile));
	if (NULL == mmp) {
		return NULL;
	}

	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		mmpfile_close(mmp);
		return NULL;
	}

	le2cpu32s(&signature);
	if (MMP_SIGNATURE != signature) {
		mmpfile_close(mmp);
		return NULL;
	}

	uint32_t unknown;
	if (1 != memfile_read(&mmp->width, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->height, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->mipmap_count_or_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->type, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->rgb_bit_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->a_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->r_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->g_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mmp->b_bit_mask, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&unknown, sizeof(uint32_t), 1, mem)) {
		mmpfile_close(mmp);
		return NULL;
	}

	le2cpu32s(&mmp->width);
	le2cpu32s(&mmp->height);
	le2cpu32s(&mmp->mipmap_count_or_size);
	le2cpu32s(&mmp->type);
	le2cpu32s(&mmp->rgb_bit_count);
	le2cpu32s(&mmp->a_bit_mask);
	le2cpu32s(&mmp->r_bit_mask);
	le2cpu32s(&mmp->g_bit_mask);
	le2cpu32s(&mmp->b_bit_mask);

	// TODO: check capability POWER_OF_TWO and tex max supported size

	bool ok;
	switch (mmp->type) {
	case MMP_DXT1:
	case MMP_DXT3:
		ok = mmpfile_read_dxt(mmp, mem);
		break;
	case MMP_DD:
		ok = mmpfile_read_dd(mmp, mem);
		break;
	case MMP_PNT3:
		ok = mmpfile_read_pnt3(mmp, mem);
		break;
	case MMP_PV:
		ok = mmpfile_read_pv(mmp, mem);
		break;
	case MMP_QU:
		ok = mmpfile_read_qu(mmp, mem);
		break;
	case MMP_XX:
		ok = mmpfile_read_xx(mmp, mem);
		break;
	default:
		assert(false);
		ok = false;
	}

	if (!ok) {
		mmpfile_close(mmp);
		return NULL;
	}

	return mmp;
}

GLuint mmpfile_gentex(memfile* mem)
{
	GLuint texid;
	glGenTextures(1, &texid);

	if (0 == texid) {
		return 0;
	}

	mmpfile* mmp = mmpfile_open(mem);
	if (NULL == mmp) {
		return 0;
	}

	glBindTexture(GL_TEXTURE_2D, texid);
	glTexImage2D(GL_TEXTURE_2D, 0, mmp->internal_format, mmp->width,
		mmp->height, 0, mmp->data_format, mmp->data_type, mmp->data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	mmpfile_close(mmp);
	return texid;
}

GLuint mmpfile_gentex_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		return 0;
	}

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void* data = malloc(size);
	fread(data, 1, size, file);

	memfile* mem = memfile_open(data, size, "rb");

	GLuint texid = mmpfile_gentex(mem);

	memfile_close(mem);

	return texid;
}
