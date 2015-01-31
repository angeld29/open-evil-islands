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

/*
 *  doc/formats/mmp.txt
 */

#ifndef CE_MMPFILE_HPP
#define CE_MMPFILE_HPP

#include <stddef.h>
#include <stdint.h>

#include "memfile.hpp"
#include "resfile.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CE_MMPFILE_VERSION = 1
};

typedef enum {
    CE_MMPFILE_FORMAT_UNKNOWN,
    // standard EI formats
    CE_MMPFILE_FORMAT_DXT1,
    CE_MMPFILE_FORMAT_DXT3,
    CE_MMPFILE_FORMAT_PNT3,
    CE_MMPFILE_FORMAT_R5G6B5,
    CE_MMPFILE_FORMAT_A1RGB5,
    CE_MMPFILE_FORMAT_ARGB4,
    CE_MMPFILE_FORMAT_ARGB8,
    // CE extensions
    CE_MMPFILE_FORMAT_RGB5A1,
    CE_MMPFILE_FORMAT_RGBA4,
    CE_MMPFILE_FORMAT_RGBA8,
    CE_MMPFILE_FORMAT_R8G8B8A8,
    CE_MMPFILE_FORMAT_YCBCR,
    CE_MMPFILE_FORMAT_COUNT
} ce_mmpfile_format;

typedef struct {
    // standard EI header
    uint32_t width, height, mipmap_count;
    ce_mmpfile_format format;
    uint32_t bit_count;
    uint32_t amask, rmask, gmask, bmask;
    uint32_t ashift, rshift, gshift, bshift;
    uint32_t acount, rcount, gcount, bcount;
    uint32_t user_data_offset;
    void* texels;
    // CE extensions
    uint32_t version;
    uint32_t user_info;
    // private data
    size_t size;
    void* data;
} ce_mmpfile;

extern size_t ce_mmpfile_storage_size(unsigned int width, unsigned int height,
                            unsigned int mipmap_count, ce_mmpfile_format format);

extern ce_mmpfile* ce_mmpfile_new(unsigned int width, unsigned int height,
    unsigned int mipmap_count, ce_mmpfile_format format, unsigned int user_info);
extern ce_mmpfile* ce_mmpfile_new_data(void* data, size_t size);
extern ce_mmpfile* ce_mmpfile_new_mem_file(ce_mem_file* mem_file);
extern ce_mmpfile* ce_mmpfile_new_res_file(ce_res_file* res_file, size_t index);
extern void ce_mmpfile_del(ce_mmpfile* mmpfile);

extern void ce_mmpfile_save(const ce_mmpfile* mmpfile, const char* path);
extern void ce_mmpfile_convert(ce_mmpfile* mmpfile, ce_mmpfile_format format);
extern void ce_mmpfile_convert2(ce_mmpfile* mmpfile, ce_mmpfile* other);

#ifdef __cplusplus
}
#endif

#endif /* CE_MMPFILE_HPP */
