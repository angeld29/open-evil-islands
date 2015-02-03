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

#ifndef CE_TEXTURE_HPP
#define CE_TEXTURE_HPP

#include "atomic.hpp"
#include "string.hpp"
#include "mmpfile.hpp"

typedef enum {
    CE_TEXTURE_WRAP_REPEAT,
    CE_TEXTURE_WRAP_CLAMP,
    CE_TEXTURE_WRAP_CLAMP_TO_EDGE,
    CE_TEXTURE_WRAP_COUNT
} ce_texture_wrap_mode;

typedef struct {
    int ref_count;
    ce_string* name;
    unsigned int width, height;
    void* impl;
} ce_texture;

extern ce_texture* ce_texture_new(const char* name, ce_mmpfile* mmpfile);
extern void ce_texture_del(ce_texture* texture);

extern bool ce_texture_is_valid(const ce_texture* texture);
extern bool ce_texture_is_equal(const ce_texture* texture, const ce_texture* other);

extern void ce_texture_replace(ce_texture* texture, ce_mmpfile* mmpfile);
extern void ce_texture_wrap(ce_texture* texture, ce_texture_wrap_mode mode);

extern void ce_texture_bind(ce_texture* texture);
extern void ce_texture_unbind(ce_texture* texture);

static inline ce_texture* ce_texture_add_ref(ce_texture* texture)
{
    ce_atomic_inc(int, &texture->ref_count);
    return texture;
}

#endif
