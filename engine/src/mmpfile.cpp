/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include <algorithm>

#include <squish.h>

#include "alloc.hpp"
#include "utility.hpp"
#include "byteorder.hpp"
#include "mmpfile.hpp"

namespace cursedearth
{
    const uint32_t CE_MMPFILE_SIGNATURE = 0x504d4d;
    const uint32_t CE_MMPFILE_SIGNATURE_EXT = 0x45434d4d;

    enum {
        CE_MMPFILE_HEADER_SIZE = 76,
    };

    const uint32_t ce_mmpfile_format_signatures[CE_MMPFILE_FORMAT_COUNT] = {
        0x0, 0x31545844, 0x33545844, 0x33544e50, 0x5650, 0x5551, 0x4444,
        0x8888, 0x45431555, 0x45434444, 0x45438888, 0x45435442, 0x4543554c
    };

    ce_mmpfile_format ce_mmpfile_format_find(uint32_t signature)
    {
        for (int i = 0; i < CE_MMPFILE_FORMAT_COUNT; ++i) {
            if (ce_mmpfile_format_signatures[i] == signature) {
                return static_cast<ce_mmpfile_format>(i);
            }
        }
        return CE_MMPFILE_FORMAT_UNKNOWN;
    }

    const uint32_t ce_mmpfile_bit_counts[CE_MMPFILE_FORMAT_COUNT] = {
        0, 4, 8, 0, 16, 16, 16, 32, 16, 16, 32, 32, 24
    };

    void ce_mmpfile_write_header_null(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0x0; mmpfile->ashift = 0; mmpfile->acount = 0;
        mmpfile->rmask = 0x0; mmpfile->rshift = 0; mmpfile->rcount = 0;
        mmpfile->gmask = 0x0; mmpfile->gshift = 0; mmpfile->gcount = 0;
        mmpfile->bmask = 0x0; mmpfile->bshift = 0; mmpfile->bcount = 0;
    }

    void ce_mmpfile_write_header_dxt1(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0x8000; mmpfile->ashift = 15; mmpfile->acount = 1;
        mmpfile->rmask = 0x7c00; mmpfile->rshift = 10; mmpfile->rcount = 5;
        mmpfile->gmask = 0x3e0;  mmpfile->gshift = 5;  mmpfile->gcount = 5;
        mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
    }

    void ce_mmpfile_write_header_dxt3(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0xf000; mmpfile->ashift = 12; mmpfile->acount = 4;
        mmpfile->rmask = 0xf00;  mmpfile->rshift = 8;  mmpfile->rcount = 4;
        mmpfile->gmask = 0xf0;   mmpfile->gshift = 4;  mmpfile->gcount = 4;
        mmpfile->bmask = 0xf;    mmpfile->bshift = 0;  mmpfile->bcount = 4;
    }

    void ce_mmpfile_write_header_r5g6b5(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0x0;    mmpfile->ashift = 0;  mmpfile->acount = 0;
        mmpfile->rmask = 0xf800; mmpfile->rshift = 11; mmpfile->rcount = 5;
        mmpfile->gmask = 0x7e0;  mmpfile->gshift = 5;  mmpfile->gcount = 6;
        mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
    }

    void ce_mmpfile_write_header_a1rgb5(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0x8000; mmpfile->ashift = 15; mmpfile->acount = 1;
        mmpfile->rmask = 0x7c00; mmpfile->rshift = 10; mmpfile->rcount = 5;
        mmpfile->gmask = 0x3e0;  mmpfile->gshift = 5;  mmpfile->gcount = 5;
        mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
    }

    void ce_mmpfile_write_header_argb4(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0xf000; mmpfile->ashift = 12; mmpfile->acount = 4;
        mmpfile->rmask = 0xf00;  mmpfile->rshift = 8;  mmpfile->rcount = 4;
        mmpfile->gmask = 0xf0;   mmpfile->gshift = 4;  mmpfile->gcount = 4;
        mmpfile->bmask = 0xf;    mmpfile->bshift = 0;  mmpfile->bcount = 4;
    }

    void ce_mmpfile_write_header_argb8(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0xff000000; mmpfile->ashift = 24; mmpfile->acount = 8;
        mmpfile->rmask = 0xff0000;   mmpfile->rshift = 16; mmpfile->rcount = 8;
        mmpfile->gmask = 0xff00;     mmpfile->gshift = 8;  mmpfile->gcount = 8;
        mmpfile->bmask = 0xff;       mmpfile->bshift = 0;  mmpfile->bcount = 8;
    }

    void ce_mmpfile_write_header_rgb5a1(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0x1f;   mmpfile->ashift = 0;  mmpfile->acount = 5;
        mmpfile->rmask = 0x8000; mmpfile->rshift = 15; mmpfile->rcount = 1;
        mmpfile->gmask = 0x7c00; mmpfile->gshift = 10; mmpfile->gcount = 5;
        mmpfile->bmask = 0x3e0;  mmpfile->bshift = 5;  mmpfile->bcount = 5;
    }

    void ce_mmpfile_write_header_rgba4(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0xf;    mmpfile->ashift = 0;  mmpfile->acount = 4;
        mmpfile->rmask = 0xf000; mmpfile->rshift = 12; mmpfile->rcount = 4;
        mmpfile->gmask = 0xf00;  mmpfile->gshift = 8;  mmpfile->gcount = 4;
        mmpfile->bmask = 0xf0;   mmpfile->bshift = 4;  mmpfile->bcount = 4;
    }

    void ce_mmpfile_write_header_rgba8(ce_mmpfile* mmpfile)
    {
        mmpfile->amask = 0xff;       mmpfile->ashift = 0;  mmpfile->acount = 8;
        mmpfile->rmask = 0xff000000; mmpfile->rshift = 24; mmpfile->rcount = 8;
        mmpfile->gmask = 0xff0000;   mmpfile->gshift = 16; mmpfile->gcount = 8;
        mmpfile->bmask = 0xff00;     mmpfile->bshift = 8;  mmpfile->bcount = 8;
    }

    void (*ce_mmpfile_write_header_procs[CE_MMPFILE_FORMAT_COUNT])(ce_mmpfile*) = {
        ce_mmpfile_write_header_null,
        ce_mmpfile_write_header_dxt1,
        ce_mmpfile_write_header_dxt3,
        ce_mmpfile_write_header_null,
        ce_mmpfile_write_header_r5g6b5,
        ce_mmpfile_write_header_a1rgb5,
        ce_mmpfile_write_header_argb4,
        ce_mmpfile_write_header_argb8,
        ce_mmpfile_write_header_rgb5a1,
        ce_mmpfile_write_header_rgba4,
        ce_mmpfile_write_header_rgba8,
        ce_mmpfile_write_header_null,
        ce_mmpfile_write_header_null
    };

    inline size_t ce_mmpfile_storage_size_generic(unsigned int width, unsigned int height, ce_mmpfile_format format)
    {
        return ce_mmpfile_bit_counts[format] * width * height / 8;
    }

    size_t ce_mmpfile_storage_size_dxt(unsigned int width, unsigned int height, ce_mmpfile_format format)
    {
        // special case for DXT format that have a block of fixed size: 8 for DXT1, 16 for DXT3
        return std::max<size_t>(2 * ce_mmpfile_bit_counts[format], ce_mmpfile_storage_size_generic(width, height, format));
    }

    size_t ce_mmpfile_storage_size_ycbcr(unsigned int width, unsigned int height, ce_mmpfile_format)
    {
        return 3 * width * height / 2;
    }

    size_t (*ce_mmpfile_storage_size_procs[CE_MMPFILE_FORMAT_COUNT])(unsigned int width, unsigned int height, ce_mmpfile_format format) = {
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_dxt,
        ce_mmpfile_storage_size_dxt,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_generic,
        ce_mmpfile_storage_size_ycbcr
    };

    size_t ce_mmpfile_storage_size(unsigned int width, unsigned int height, unsigned int mipmap_count, ce_mmpfile_format format)
    {
        size_t size = 0;
        for (unsigned int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
            size += (*ce_mmpfile_storage_size_procs[format])(width, height, format);
        }
        return size;
    }

    ce_mmpfile* ce_mmpfile_new(unsigned int width, unsigned int height, unsigned int mipmap_count, ce_mmpfile_format format, unsigned int user_info)
    {
        ce_mmpfile* mmpfile = (ce_mmpfile*)ce_alloc(sizeof(ce_mmpfile));
        mmpfile->width = width;
        mmpfile->height = height;
        mmpfile->mipmap_count = mipmap_count;
        mmpfile->format = format;
        mmpfile->bit_count = ce_mmpfile_bit_counts[format];
        mmpfile->size = ce_mmpfile_storage_size(width, height, mipmap_count, format);
        mmpfile->user_data_offset = CE_MMPFILE_HEADER_SIZE + mmpfile->size;
        mmpfile->data = ce_alloc(mmpfile->size);
        mmpfile->texels = mmpfile->data;
        mmpfile->version = CE_MMPFILE_VERSION;
        mmpfile->user_info = user_info;
        (*ce_mmpfile_write_header_procs[format])(mmpfile);
        return mmpfile;
    }

    ce_mmpfile* ce_mmpfile_new_data(void* data, size_t size)
    {
        union {
            uint8_t* u8;
            uint32_t* u32;
        } ptr = { static_cast<uint8_t*>(data) };

        uint32_t signature = le2cpu(*ptr.u32++);
        assert(CE_MMPFILE_SIGNATURE == signature && "wrong signature");

        ce_mmpfile* mmpfile = (ce_mmpfile*)ce_alloc(sizeof(ce_mmpfile));
        mmpfile->width = le2cpu(*ptr.u32++);
        mmpfile->height = le2cpu(*ptr.u32++);
        mmpfile->mipmap_count = le2cpu(*ptr.u32++);
        mmpfile->format = ce_mmpfile_format_find(le2cpu(*ptr.u32++));
        mmpfile->bit_count = le2cpu(*ptr.u32++);
        mmpfile->amask = le2cpu(*ptr.u32++);
        mmpfile->ashift = le2cpu(*ptr.u32++);
        mmpfile->acount = le2cpu(*ptr.u32++);
        mmpfile->rmask = le2cpu(*ptr.u32++);
        mmpfile->rshift = le2cpu(*ptr.u32++);
        mmpfile->rcount = le2cpu(*ptr.u32++);
        mmpfile->gmask = le2cpu(*ptr.u32++);
        mmpfile->gshift = le2cpu(*ptr.u32++);
        mmpfile->gcount = le2cpu(*ptr.u32++);
        mmpfile->bmask = le2cpu(*ptr.u32++);
        mmpfile->bshift = le2cpu(*ptr.u32++);
        mmpfile->bcount = le2cpu(*ptr.u32++);
        mmpfile->user_data_offset = le2cpu(*ptr.u32++);
        mmpfile->texels = ptr.u8;
        mmpfile->version = 0;
        mmpfile->user_info = 0;
        mmpfile->size = size;
        mmpfile->data = data;
        if (mmpfile->user_data_offset > CE_MMPFILE_HEADER_SIZE) {
            ptr.u8 += mmpfile->user_data_offset - CE_MMPFILE_HEADER_SIZE;
            if (CE_MMPFILE_SIGNATURE_EXT == le2cpu(*ptr.u32++)) {
                mmpfile->version = le2cpu(*ptr.u32++);
                mmpfile->user_info = le2cpu(*ptr.u32);
            }
        }
        return mmpfile;
    }

    ce_mmpfile* ce_mmpfile_new_mem_file(ce_mem_file* mem_file)
    {
        size_t size = ce_mem_file_size(mem_file);
        void* data = ce_alloc(size);
        ce_mem_file_read(mem_file, data, 1, size);
        return ce_mmpfile_new_data(data, size);
    }

    ce_mmpfile* ce_mmpfile_new_res_file(ce_res_file* res_file, size_t index)
    {
        return ce_mmpfile_new_data(ce_res_file_node_data(res_file, index), ce_res_file_node_size(res_file, index));
    }

    void ce_mmpfile_del(ce_mmpfile* mmpfile)
    {
        if (NULL != mmpfile) {
            ce_free(mmpfile->data, mmpfile->size);
            ce_free(mmpfile, sizeof(ce_mmpfile));
        }
    }

    void ce_mmpfile_save(const ce_mmpfile* mmpfile, const char* path)
    {
        // compressed PNT3 not supported
        assert(CE_MMPFILE_FORMAT_PNT3 != mmpfile->format);
        if (CE_MMPFILE_FORMAT_PNT3 == mmpfile->format) {
            return;
        }

        FILE* file = fopen(path, "wb");
        if (NULL != file) {
            uint32_t header[22];
            header[0] = cpu2le(CE_MMPFILE_SIGNATURE);
            header[1] = cpu2le(mmpfile->width);
            header[2] = cpu2le(mmpfile->height);
            header[3] = cpu2le(mmpfile->mipmap_count);
            header[4] = cpu2le(ce_mmpfile_format_signatures[mmpfile->format]);
            header[5] = cpu2le(mmpfile->bit_count);
            header[6] = cpu2le(mmpfile->amask);
            header[7] = cpu2le(mmpfile->ashift);
            header[8] = cpu2le(mmpfile->acount);
            header[9] = cpu2le(mmpfile->rmask);
            header[10] = cpu2le(mmpfile->rshift);
            header[11] = cpu2le(mmpfile->rcount);
            header[12] = cpu2le(mmpfile->gmask);
            header[13] = cpu2le(mmpfile->gshift);
            header[14] = cpu2le(mmpfile->gcount);
            header[15] = cpu2le(mmpfile->bmask);
            header[16] = cpu2le(mmpfile->bshift);
            header[17] = cpu2le(mmpfile->bcount);
            header[18] = cpu2le(mmpfile->user_data_offset);
            header[19] = cpu2le(CE_MMPFILE_SIGNATURE_EXT);
            header[20] = cpu2le(mmpfile->version);
            header[21] = cpu2le(mmpfile->user_info);
            fwrite(header, 4, 19, file);
            fwrite(mmpfile->texels, 1, ce_mmpfile_storage_size(mmpfile->width, mmpfile->height, mmpfile->mipmap_count, mmpfile->format), file);
            fwrite(header + 19, 4, 3, file);
            fclose(file);
        }
    }

    void ce_mmpfile_argb_swap16_rgba(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        uint16_t* dst = static_cast<uint16_t*>(other->texels);
        const uint16_t* src = static_cast<const uint16_t*>(mmpfile->texels);
        for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height; i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
            for (const uint16_t* end = src + width * height; src != end; ++src) {
                *dst++ = *src << mmpfile->acount | *src >> mmpfile->ashift;
            }
        }
    }

    void ce_mmpfile_argb_swap32_rgba(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        uint32_t* dst = static_cast<uint32_t*>(other->texels);
        const uint32_t* src = static_cast<uint32_t*>(mmpfile->texels);
        for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height; i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
            for (const uint32_t* end = src + width * height; src != end; ++src) {
                *dst++ = *src << mmpfile->acount | *src >> mmpfile->ashift;
            }
        }
    }

    void ce_mmpfile_unpack16(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        uint8_t* dst = static_cast<uint8_t*>(other->texels);
        const uint16_t* src = static_cast<const uint16_t*>(mmpfile->texels);
        for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height; i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
            for (const uint16_t* end = src + width * height; src != end; ++src) {
                *dst++ = ((*src & mmpfile->rmask) >> mmpfile->rshift) * 255 / (mmpfile->rmask >> mmpfile->rshift);
                *dst++ = ((*src & mmpfile->gmask) >> mmpfile->gshift) * 255 / (mmpfile->gmask >> mmpfile->gshift);
                *dst++ = ((*src & mmpfile->bmask) >> mmpfile->bshift) * 255 / (mmpfile->bmask >> mmpfile->bshift);
                *dst++ = 0 == mmpfile->amask ? 255 : ((*src & mmpfile->amask) >> mmpfile->ashift) * 255 / (mmpfile->amask >> mmpfile->ashift);
            }
        }
    }

    void ce_mmpfile_unpack32(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        uint8_t* dst = static_cast<uint8_t*>(other->texels);
        const uint32_t* src = static_cast<const uint32_t*>(mmpfile->texels);
        for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height; i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
            for (const uint32_t* end = src + width * height; src != end; ++src) {
                *dst++ = ((*src & mmpfile->rmask) >> mmpfile->rshift) * 255 / (mmpfile->rmask >> mmpfile->rshift);
                *dst++ = ((*src & mmpfile->gmask) >> mmpfile->gshift) * 255 / (mmpfile->gmask >> mmpfile->gshift);
                *dst++ = ((*src & mmpfile->bmask) >> mmpfile->bshift) * 255 / (mmpfile->bmask >> mmpfile->bshift);
                *dst++ = 0 == mmpfile->amask ? 255 : ((*src & mmpfile->amask) >> mmpfile->ashift) * 255 / (mmpfile->amask >> mmpfile->ashift);
            }
        }
    }

    void ce_mmpfile_convert_unknown(const ce_mmpfile*, ce_mmpfile*)
    {
        assert(false && "not implemented");
    }

    void ce_mmpfile_convert_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");

        uint8_t* rgba = static_cast<uint8_t*>(other->texels);
        const uint8_t* blocks = static_cast<const uint8_t*>(mmpfile->texels);
        int flags = CE_MMPFILE_FORMAT_DXT3 == mmpfile->format ? squish::kDxt3 : squish::kDxt1;

        for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height; i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
            squish::DecompressImage(rgba, width, height, blocks, flags);
            rgba += ce_mmpfile_storage_size(width, height, 1, other->format);
            blocks += ce_mmpfile_storage_size(width, height, 1, mmpfile->format);
        }
    }

    // PNT3 is just a simple RLE (Run-Length Encoding)
    void ce_mmpfile_decompress_pnt3(uint8_t* dst, const uint32_t* src, size_t size)
    {
        assert(0 == size % sizeof(uint32_t));

        const uint32_t* end = src + size / sizeof(uint32_t);

        size_t n = 0;
        uint32_t v;

        while (src != end) {
            v = le2cpu(*src++);
            if (v > 1000000U || 0 == v) {
                ++n;
            } else {
                memcpy(dst, src - 1 - n, n * sizeof(uint32_t));
                dst += n * sizeof(uint32_t);
                memset(dst, 0, v);
                dst += v;
                n = 0;
            }
        }

        memcpy(dst, src - n, n * sizeof(uint32_t));
    }

    void ce_mmpfile_convert_pnt3(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        assert(CE_MMPFILE_FORMAT_ARGB8 == other->format && "not implemented");
        // mipmap_count == compressed size for pnt3
        if (mmpfile->mipmap_count < other->size) {
            // pnt3 compressed
            ce_mmpfile_decompress_pnt3(static_cast<uint8_t*>(other->texels), static_cast<const uint32_t*>(mmpfile->texels), mmpfile->mipmap_count);
        } else {
            memcpy(other->texels, mmpfile->texels, other->size);
        }
    }

    void ce_mmpfile_convert_r5g6b5(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");
        ce_mmpfile_unpack16(mmpfile, other);
    }

    void ce_mmpfile_convert_a1rgb5(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        if (CE_MMPFILE_FORMAT_RGB5A1 == other->format) {
            ce_mmpfile_argb_swap16_rgba(mmpfile, other);
        } else if (CE_MMPFILE_FORMAT_R8G8B8A8 == other->format) {
            ce_mmpfile_unpack16(mmpfile, other);
        } else {
            assert(false && "not implemented");
        }
    }

    void ce_mmpfile_convert_argb4(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        if (CE_MMPFILE_FORMAT_RGBA4 == other->format) {
            ce_mmpfile_argb_swap16_rgba(mmpfile, other);
        } else if (CE_MMPFILE_FORMAT_R8G8B8A8 == other->format) {
            ce_mmpfile_unpack16(mmpfile, other);
        } else {
            assert(false && "not implemented");
        }
    }

    void ce_mmpfile_convert_argb8(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        if (CE_MMPFILE_FORMAT_RGBA8 == other->format) {
            ce_mmpfile_argb_swap32_rgba(mmpfile, other);
        } else if (CE_MMPFILE_FORMAT_R8G8B8A8 == other->format) {
            ce_mmpfile_unpack32(mmpfile, other);
        } else {
            assert(false && "not implemented");
        }
    }

    void ce_mmpfile_convert_r8g8b8a8(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        assert((CE_MMPFILE_FORMAT_DXT1 == other->format || CE_MMPFILE_FORMAT_DXT3 == other->format) && "not implemented");

        const uint8_t* rgba = static_cast<const uint8_t*>(mmpfile->texels);
        uint8_t* blocks = static_cast<uint8_t*>(other->texels);
        int flags = squish::kColourRangeFit | (CE_MMPFILE_FORMAT_DXT3 == other->format ? squish::kDxt3 : squish::kDxt1);

        for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height; i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
            squish::CompressImage(rgba, width, height, blocks, flags);
            rgba += ce_mmpfile_storage_size(width, height, 1, mmpfile->format);
            blocks += ce_mmpfile_storage_size(width, height, 1, other->format);
        }
    }

    void ce_mmpfile_convert_ycbcr(const ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");

        const uint8_t* y_data = static_cast<const uint8_t*>(mmpfile->texels);
        const uint8_t* cb_data = y_data + mmpfile->width * mmpfile->height;
        const uint8_t* cr_data = cb_data + (mmpfile->width / 2) * (mmpfile->height / 2);

        uint8_t* texels = static_cast<uint8_t*>(other->texels);

        for (unsigned int h = 0; h < mmpfile->height; ++h) {
            for (unsigned int w = 0; w < mmpfile->width; ++w) {
                size_t index = 4 * (h * mmpfile->width + w);

                int y = 298 * (y_data[h * mmpfile->width + w] - 16);
                int cb = cb_data[(h / 2) * (mmpfile->width / 2) + w / 2] - 128;
                int cr = cr_data[(h / 2) * (mmpfile->width / 2) + w / 2] - 128;

                texels[index + 0] = clamp((y + 409 * cr + 128) / 256, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
                texels[index + 1] = clamp((y - 100 * cb - 208 * cr + 128) / 256, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
                texels[index + 2] = clamp((y + 516 * cb + 128) / 256, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
                texels[index + 3] = std::numeric_limits<uint8_t>::max();
            }
        }
    }

    void (*ce_mmpfile_convert_procs[CE_MMPFILE_FORMAT_COUNT])(const ce_mmpfile*, ce_mmpfile*) = {
        ce_mmpfile_convert_unknown,
        ce_mmpfile_convert_dxt,
        ce_mmpfile_convert_dxt,
        ce_mmpfile_convert_pnt3,
        ce_mmpfile_convert_r5g6b5,
        ce_mmpfile_convert_a1rgb5,
        ce_mmpfile_convert_argb4,
        ce_mmpfile_convert_argb8,
        ce_mmpfile_convert_unknown,
        ce_mmpfile_convert_unknown,
        ce_mmpfile_convert_unknown,
        ce_mmpfile_convert_r8g8b8a8,
        ce_mmpfile_convert_ycbcr
    };

    void ce_mmpfile_convert(ce_mmpfile* mmpfile, ce_mmpfile_format format)
    {
        unsigned int mipmap_count = mmpfile->mipmap_count;

        // special case for pnt3
        if (CE_MMPFILE_FORMAT_PNT3 == mmpfile->format) {
            mipmap_count = 1;
        }

        ce_mmpfile* other = ce_mmpfile_new(mmpfile->width, mmpfile->height, mipmap_count, format, mmpfile->user_info);
        (*ce_mmpfile_convert_procs[mmpfile->format])(mmpfile, other);

        ce_mmpfile temp = *mmpfile;
        *mmpfile = *other;
        *other = temp;

        ce_mmpfile_del(other);
    }

    void ce_mmpfile_convert2(ce_mmpfile* mmpfile, ce_mmpfile* other)
    {
        (*ce_mmpfile_convert_procs[mmpfile->format])(mmpfile, other);
    }
}
