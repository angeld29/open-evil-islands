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

#define IS_MUL4(x)     (((x) & 3) == 0)

#define GETL16(buf) \
   (((unsigned short)(buf)[0]     ) | \
    ((unsigned short)(buf)[1] << 8))

static int mul8bit(int a, int b)
{
   int t = a * b + 128;
   return((t + (t >> 8)) >> 8);
}

static int blerp(int a, int b, int x)
{
   return(a + mul8bit(b - a, x));
}

/* unpack RGB565 to BGR */
static void unpack_rgb565(unsigned char *dst, unsigned short v)
{
   int r = (v >> 11) & 0x1f;
   int g = (v >>  5) & 0x3f;
   int b = (v      ) & 0x1f;
   
   dst[0] = (b << 3) | (b >> 2);
   dst[1] = (g << 2) | (g >> 4);
   dst[2] = (r << 3) | (r >> 2);
}

static void lerp_rgb(unsigned char *dst, unsigned char *a, unsigned char *b, int f)
{
   dst[0] = blerp(a[0], b[0], f);
   dst[1] = blerp(a[1], b[1], f);
   dst[2] = blerp(a[2], b[2], f);
}

static void decode_color_block(unsigned char *dst, unsigned char *src,
                               int w, int h, int rowbytes, int format)
{
   int i, x, y;
   unsigned int indexes, idx;
   unsigned char *d;
   unsigned char colors[4][3];
   unsigned short c0, c1;
   
   c0 = GETL16(&src[0]);
   c1 = GETL16(&src[2]);

   unpack_rgb565(colors[0], c0);
   unpack_rgb565(colors[1], c1);
   
   if(c0 > c1)
   {
      lerp_rgb(colors[2], colors[0], colors[1], 0x55);
      lerp_rgb(colors[3], colors[0], colors[1], 0xaa);
   }
   else
   {
      for(i = 0; i < 3; ++i)
      {
         colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
         colors[3][i] = 255;
      }
   }
   
   src += 4;
   for(y = 0; y < h; ++y)
   {
      d = dst + (y * rowbytes);
      indexes = src[y];
      for(x = 0; x < w; ++x)
      {
         idx = indexes & 0x03;
         d[0] = colors[idx][2];
         d[1] = colors[idx][1];
         d[2] = colors[idx][0];
         if(MMP_DXT1 == format)
            d[3] = ((c0 <= c1) && idx == 3) ? 0 : 255;
         indexes >>= 2;
         d += 4;
      }
   }
}

static void decode_alpha_block_DXT3(unsigned char *dst, unsigned char *src,
                                    int w, int h, int rowbytes)
{
   int x, y;
   unsigned char *d;
   unsigned int bits;
   
   for(y = 0; y < h; ++y)
   {
      d = dst + (y * rowbytes);
      bits = GETL16(&src[2 * y]);
      for(x = 0; x < w; ++x)
      {
         d[0] = (bits & 0x0f) * 17;
         bits >>= 4;
         d += 4;
      }
   }
}

static int dxt_decompress(unsigned char *dst, unsigned char *src, int format,
                   unsigned int width, unsigned int height)
{
   unsigned char *d, *s;
   unsigned int x, y, sx, sy;
   
   if(!(IS_MUL4(width) && IS_MUL4(height)))
      return(0);
   
   sx = (width  < 4) ? width  : 4;
   sy = (height < 4) ? height : 4;
   
   s = src;

   for(y = 0; y < height; y += 4)
   {
      for(x = 0; x < width; x += 4)
      {
         d = dst + (y * width + x) * 4;
         if(MMP_DXT1 == format)
         {
            decode_color_block(d, s, sx, sy, width * 4, format);
            s += 8;
         }
         else if(MMP_DXT3 == format)
         {
            decode_alpha_block_DXT3(d + 3, s, sx, sy, width * 4);
            s += 8;
            decode_color_block(d, s, sx, sy, width * 4, format);
            s += 8;
         }
      }
   }
   
   return(1);
}

static uint8_t color_bits(uint32_t mask)
{
	uint8_t i = 0;
	while (mask) {
		if (mask & 1) {
			++i;
		}
		mask >>= 1;
	}
	return i;
}

static uint8_t color_shift(uint32_t mask)
{
	if (!mask) {
		return 0;
	}
	uint8_t i = 0;
	while (!((mask >> i) & 1)) {
		++i;
	}
	return i;
}

static bool mmpfile_read_dd(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));
	assert(0xf00 == mmp->r_bit_mask);
	assert(0xf0 == mmp->g_bit_mask);
	assert(0xf == mmp->b_bit_mask);
	assert(0xf000 == mmp->a_bit_mask);

	size_t linear_size = mmp->width * mmp->height * 2;

	mmp->data = malloc(linear_size);
	if (NULL == mmp->data) {
		return false;
	}

	if (1 != memfile_read(mmp->data, linear_size, 1, mem)) {
		return false;
	}

	// ARGB -> RGBA
	/*for (size_t i = 0; i < linear_size; i += 2) {
		uint16_t* value = (uint16_t*)(mmp->data + i);
		uint16_t r_bit = (*value & 0xf00) >> 8;
		uint16_t g_bit = (*value & 0xf0) >> 4;
		uint16_t b_bit = *value & 0xf;
		uint16_t a_bit = (*value & 0xf000) >> 12;
		*value = (r_bit << 12) | (g_bit << 8) | (b_bit << 4) | a_bit;
	}*/

	mmp->internal_format = GL_RGBA;
	mmp->data_format = GL_BGRA;
	mmp->data_type = GL_UNSIGNED_SHORT_4_4_4_4_REV;

	return true;
}

static bool mmpfile_read_dxt(mmpfile* mmp, memfile* mem)
{
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

	// TODO: refactoring
	mmp->data = calloc(mmp->width * mmp->height * 4, 1);
	if (NULL == mmp->data) {
		free(buffer);
		return false;
	}

	for (size_t y = 0; y < mmp->height; ++y) {
		for (size_t x = 0; x < mmp->width; ++x) {
			mmp->data[y * (mmp->width * 4) + x + 3] = 255;
		}
	}

	dxt_decompress(mmp->data, buffer, mmp->type, mmp->width, mmp->height);

	mmp->internal_format = GL_RGBA;
	mmp->data_format = GL_RGBA;
	mmp->data_type = GL_UNSIGNED_BYTE;

	printf("r %#x\n", mmp->r_bit_mask);
	printf("g %#x\n", mmp->g_bit_mask);
	printf("b %#x\n", mmp->b_bit_mask);
	printf("a %#x\n", mmp->a_bit_mask);

	free(buffer);
	return true;
}

/*static void pnt2dds(mmpheader* mmp, ddsheader* dds)
{
---read_dds---
d.bpp = 4
bpp != 2
not alpha only image
---load_layer---
bpp >= 3
else

	mmpheader mmp = mmp->mmp.mmp;
	memset(&mmp->hdr.dds, '\0', sizeof(ddsheader));

	mmp->hdr.dds.size = DDS_HEADER_SIZE;
	mmp->hdr.dds.flags = DDS_CAPS | DDS_HEIGHT | DDS_WIDTH | DDS_PIXELFORMAT | DDS_PITCH;
	mmp->hdr.dds.height = hdr.height;
	mmp->hdr.dds.width = hdr.width;
	hdr.pitch_or_linsize = hdr.height * hdr.width * (hdr.rgb_bit_count >> 3);
	mmp->hdr.dds.pitch_or_linear_size = hdr.mipmap_count_or_size;
	mmp->hdr.dds.ddpf.size = DDS_PIXELFORMAT_SIZE;
	mmp->hdr.dds.ddpf.flags = DDPF_ALPHAPIXELS | DDPF_RGB;
	mmp->hdr.dds.ddpf.rgb_bit_count = hdr.rgb_bit_count;
	mmp->hdr.dds.ddpf.r_bit_mask = 0x00ff0000;
	mmp->hdr.dds.ddpf.g_bit_mask = 0x0000ff00;
	mmp->hdr.dds.ddpf.b_bit_mask = 0x000000ff;
	mmp->hdr.dds.ddpf.a_bit_mask = 0xff000000;
	mmp->hdr.dds.caps = DDS_CAPS_TEXTURE;

	if (hdr.mipmap_count_or_size < 1024 * 256) {
		assert(false && "Compressed data");
	}
}*/

static bool mmpfile_read_pv(mmpfile* mmp, memfile* mem)
{
	assert(2 == (mmp->rgb_bit_count >> 3));

	size_t linear_size = mmp->width * mmp->height * 2;

	mmp->data = malloc(linear_size);
	if (NULL == mmp->data) {
		return false;
	}

	if (1 != memfile_read(mmp->data, linear_size, 1, mem)) {
		return false;
	}

	mmp->internal_format = GL_RGB;
	mmp->data_format = GL_RGB;
	mmp->data_type = GL_UNSIGNED_SHORT_5_6_5;

	return true;
}

static bool mmpfile_read_qu(mmpfile* mmp, memfile* mem)
{
	// TODO: RGB5A1

	assert(2 == (mmp->rgb_bit_count >> 3));

	size_t linear_size = mmp->width * mmp->height * 2;

	mmp->data = malloc(linear_size);
	if (NULL == mmp->data) {
		return false;
	}

	if (1 != memfile_read(mmp->data, linear_size, 1, mem)) {
		return false;
	}

	mmp->internal_format = GL_RGBA;
	mmp->data_format = GL_RGBA;
	mmp->data_type = GL_UNSIGNED_SHORT_1_5_5_5_REV;

	return true;
}

static bool mmpfile_read_xx(mmpfile* mmp, memfile* mem)
{
/*
---read_dds---
d.bpp = 4
bpp != 2
not alpha only image
---load_layer---
bpp >= 3
else
*/

	assert(4 == (mmp->rgb_bit_count >> 3));

	size_t linear_size = mmp->width * mmp->height * 4;

	mmp->data = malloc(linear_size);
	if (NULL == mmp->data) {
		return false;
	}

	if (1 != memfile_read(mmp->data, linear_size, 1, mem)) {
		return false;
	}

	mmp->internal_format = GL_RGBA;
	mmp->data_format = GL_RGBA;
	mmp->data_type = GL_UNSIGNED_INT_8_8_8_8;

	return true;
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
	case MMP_DD:
		ok = mmpfile_read_dd(mmp, mem);
		break;
	case MMP_DXT1:
	case MMP_DXT3:
		ok = mmpfile_read_dxt(mmp, mem);
		break;
	/*case MMP_PNT3:
		pnt2dds(mmp);
		break;*/
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

	GLenum err_code = glGetError();
	while( GL_NO_ERROR != err_code )
	{
		printf( "OpenGL Error: %i\n", err_code );
		err_code = glGetError();
	}

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
