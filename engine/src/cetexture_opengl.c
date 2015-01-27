/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include "celib.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "ceopengl.h"
#include "cetexture.h"

typedef struct {
    GLuint id;
} ce_texture_opengl;

static unsigned int ce_texture_correct_mipmap_count(unsigned int mipmap_count)
{
    // OpenGL Specification:
    // GL_INVALID_VALUE may be generated if level is less
    // than 0 or greater than log2(GL_MAX_TEXTURE_SIZE)

    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

    unsigned int max_level = log2f(max_texture_size);

    static bool reported; // FIXME: threads
    if (!reported && mipmap_count - 1 > max_level) {
        ce_logging_warning("texture: your hardware supports a maximum "
            "of %d mipmaps, extra mipmaps were discarded", max_level);
        reported = true;
    }

    return ce_clamp(size_t, mipmap_count, 1, max_level + 1);
}

static void ce_texture_setup_filters(unsigned int mipmap_count)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if (mipmap_count > 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                        GL_LINEAR_MIPMAP_LINEAR);
        if (GLEW_VERSION_1_2 || GLEW_SGIS_texture_lod) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count - 1);
        } else if (GLEW_SGIS_generate_mipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            static bool reported; // FIXME: threads
            if (!reported) {
                ce_logging_warning("texture: some opengl features are not "
                                    "available, mipmapping was disabled");
                reported = true;
            }
        }
    }
}

static void ce_texture_specify(unsigned int width, unsigned int height,
                            unsigned int level, GLenum internal_format,
                            GLenum data_format, GLenum data_type, void* data)
{
    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

    unsigned int new_width = ce_min(int, width, max_texture_size);
    unsigned int new_height = ce_min(int, height, max_texture_size);

    if (!GLEW_VERSION_2_0 && !GLEW_ARB_texture_non_power_of_two) {
        if (!ce_ispot(new_width)) new_width = ce_nlpot(new_width);
        if (!ce_ispot(new_height)) new_height = ce_nlpot(new_height);
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
    unsigned int mipmap_count = ce_texture_correct_mipmap_count(mmpfile->mipmap_count);
    uint8_t* src = mmpfile->texels;

    // most EI's textures of width divisible by 4 (GL's default row alignment)
    const bool not_aligned = 0 != mmpfile->width % 4;

    if (not_aligned) {
        glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
            i < mipmap_count; ++i, width >>= 1, height >>= 1) {
        ce_texture_specify(width, height, i,
            internal_format, data_format, data_type, src);
        src += ce_mmpfile_storage_size(width, height, 1, mmpfile->format);
    }

    if (not_aligned) {
        glPopClientAttrib();
    }

    ce_texture_setup_filters(mipmap_count);
}

static void ce_texture_generate_compressed(ce_mmpfile* mmpfile,
                                            GLenum internal_format)
{
    unsigned int mipmap_count = ce_texture_correct_mipmap_count(mmpfile->mipmap_count);
    const uint8_t* src = mmpfile->texels;

    for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
            i < mipmap_count; ++i, width >>= 1, height >>= 1) {
        size_t size = ce_mmpfile_storage_size(width, height, 1, mmpfile->format);
        glCompressedTexImage2D(GL_TEXTURE_2D, i,
            internal_format, width, height, 0, size, src);
        src += size;
    }

    ce_texture_setup_filters(mipmap_count);
}

static void ce_texture_generate_argb8(ce_mmpfile* mmpfile);

static void ce_texture_generate_unknown(ce_mmpfile* CE_UNUSED(mmpfile))
{
    assert(false && "not implemented");
}

static void ce_texture_generate_dxt(ce_mmpfile* mmpfile)
{
    if (GLEW_VERSION_1_3 && (GLEW_EXT_texture_compression_s3tc ||
            (CE_MMPFILE_FORMAT_DXT1 == mmpfile->format &&
            GLEW_EXT_texture_compression_dxt1))) {
        ce_texture_generate_compressed(mmpfile, (GLenum[])
            {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
            GL_COMPRESSED_RGBA_S3TC_DXT3_EXT}
            [CE_MMPFILE_FORMAT_DXT3 == mmpfile->format]);
    } else {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    }
}

static void ce_texture_generate_pnt3(ce_mmpfile* mmpfile)
{
    ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_ARGB8);
    ce_texture_generate_argb8(mmpfile);
}

static void ce_texture_generate_r5g6b5(ce_mmpfile* mmpfile)
{
    if (GLEW_VERSION_1_2) {
        ce_texture_generate(mmpfile, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
    } else {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    }
}

static void ce_texture_generate_a1rgb5(ce_mmpfile* mmpfile)
{
    if (GLEW_VERSION_1_2) {
        ce_texture_generate(mmpfile, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV);
    } else if (GLEW_EXT_packed_pixels) {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_RGB5A1);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
    } else {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    }
}

static void ce_texture_generate_argb4(ce_mmpfile* mmpfile)
{
    if (GLEW_VERSION_1_2) {
        ce_texture_generate(mmpfile, GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV);
    } else if (GLEW_EXT_packed_pixels) {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_RGBA4);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
    } else {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    }
}

static void ce_texture_generate_argb8(ce_mmpfile* mmpfile)
{
    if (GLEW_VERSION_1_2) {
        ce_texture_generate(mmpfile, GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV);
    } else if (GLEW_EXT_packed_pixels) {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_RGBA8);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8);
    } else {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
        ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    }
}

static void ce_texture_generate_a8r8g8b8(ce_mmpfile* mmpfile)
{
    ce_texture_generate(mmpfile, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
}

static void (*ce_texture_generate_procs[CE_MMPFILE_FORMAT_COUNT])(ce_mmpfile*) = {
    [CE_MMPFILE_FORMAT_UNKNOWN] = ce_texture_generate_unknown,
    [CE_MMPFILE_FORMAT_DXT1] = ce_texture_generate_dxt,
    [CE_MMPFILE_FORMAT_DXT3] = ce_texture_generate_dxt,
    [CE_MMPFILE_FORMAT_PNT3] = ce_texture_generate_pnt3,
    [CE_MMPFILE_FORMAT_R5G6B5] = ce_texture_generate_r5g6b5,
    [CE_MMPFILE_FORMAT_A1RGB5] = ce_texture_generate_a1rgb5,
    [CE_MMPFILE_FORMAT_ARGB4] = ce_texture_generate_argb4,
    [CE_MMPFILE_FORMAT_ARGB8] = ce_texture_generate_argb8,
    [CE_MMPFILE_FORMAT_RGB5A1] = ce_texture_generate_unknown,
    [CE_MMPFILE_FORMAT_RGBA4] = ce_texture_generate_unknown,
    [CE_MMPFILE_FORMAT_RGBA8] = ce_texture_generate_unknown,
    [CE_MMPFILE_FORMAT_R8G8B8A8] = ce_texture_generate_a8r8g8b8,
};

ce_texture* ce_texture_new(const char* name, ce_mmpfile* mmpfile)
{
    ce_texture* texture = ce_alloc(sizeof(ce_texture) + sizeof(ce_texture_opengl));
    ce_texture_opengl* opengl_texture = (ce_texture_opengl*)texture->impl;

    texture->ref_count = 1;
    texture->name =  ce_string_new_str(NULL != name ? name : "");

    glGenTextures(1, &opengl_texture->id);

    if (NULL != mmpfile) {
        ce_texture_replace(texture, mmpfile);
    }

    return texture;
}

void ce_texture_del(ce_texture* texture)
{
    if (NULL != texture) {
        assert(ce_atomic_fetch(int, &texture->ref_count) > 0);
        if (0 == ce_atomic_dec_and_fetch(int, &texture->ref_count)) {
            ce_texture_opengl* opengl_texture = (ce_texture_opengl*)texture->impl;
            glDeleteTextures(1, &opengl_texture->id);
            ce_string_del(texture->name);
            ce_free(texture, sizeof(ce_texture) + sizeof(ce_texture_opengl));
        }
    }
}

bool ce_texture_is_valid(const ce_texture* texture)
{
    return glIsTexture(((ce_texture_opengl*)texture->impl)->id);
}

bool ce_texture_is_equal(const ce_texture* texture, const ce_texture* other)
{
    return ((ce_texture_opengl*)texture->impl)->id ==
            ((ce_texture_opengl*)other->impl)->id;
}

void ce_texture_replace(ce_texture* texture, ce_mmpfile* mmpfile)
{
    texture->width = mmpfile->width;
    texture->height = mmpfile->height;

    ce_texture_bind(texture);
    (*ce_texture_generate_procs[mmpfile->format])(mmpfile);
    ce_texture_unbind(texture);
}

void ce_texture_wrap(ce_texture* texture, ce_texture_wrap_mode mode)
{
    ce_texture_bind(texture);
    if (CE_TEXTURE_WRAP_CLAMP_TO_EDGE == mode) {
        if (GLEW_VERSION_1_2 || GLEW_EXT_texture_edge_clamp ||
                                GLEW_SGIS_texture_edge_clamp) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
    } else if (CE_TEXTURE_WRAP_CLAMP == mode) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    ce_texture_unbind(texture);
}

void ce_texture_bind(ce_texture* texture)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ((ce_texture_opengl*)texture->impl)->id);
}

void ce_texture_unbind(ce_texture* CE_UNUSED(texture))
{
    glDisable(GL_TEXTURE_2D);
}
