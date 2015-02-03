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

#ifndef CE_SHADER_HPP
#define CE_SHADER_HPP

#include <cstddef>

#include "atomic.hpp"
#include "string.hpp"

typedef enum {
    CE_SHADER_TYPE_UNKNOWN,
    CE_SHADER_TYPE_VERTEX,
    CE_SHADER_TYPE_FRAGMENT,
    CE_SHADER_TYPE_COUNT
} ce_shader_type;

typedef struct {
    ce_shader_type shader_type;
    size_t resource_index;
} ce_shader_info;

typedef struct {
    int ref_count;
    ce_string* name;
    void* impl;
} ce_shader;

extern bool ce_shader_is_available(void);

extern ce_shader* ce_shader_new(const char* name, const ce_shader_info shader_infos[]);
extern void ce_shader_del(ce_shader* shader);

extern bool ce_shader_is_valid(const ce_shader* shader);

extern void ce_shader_bind(ce_shader* shader);
extern void ce_shader_unbind(ce_shader* shader);

static inline ce_shader* ce_shader_add_ref(ce_shader* shader)
{
    ce_atomic_inc(int, &shader->ref_count);
    return shader;
}

#endif
