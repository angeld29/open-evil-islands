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
 *  See doc/formats/figfile.txt for more details.
*/

#include <assert.h>

#include "cebyteorder.h"
#include "cealloc.h"
#include "cefigfile.h"

static float ce_figfile_value_fig1(const float* params, size_t CE_UNUSED(stride),
                                    const ce_complection* CE_UNUSED(complection))
{
    return *params;
}

static float ce_figfile_value_fig8(const float* params, size_t stride,
                                    const ce_complection* complection)
{
    float temp1 = params[0 * stride] +
        (params[1 * stride] - params[0 * stride]) * complection->strength;
    float temp2 = params[2 * stride] +
        (params[3 * stride] - params[2 * stride]) * complection->strength;
    float value = temp1 + (temp2 - temp1) * complection->dexterity;
    temp1 = params[4 * stride] +
        (params[5 * stride] - params[4 * stride]) * complection->strength;
    temp2 = params[6 * stride] +
        (params[7 * stride] - params[6 * stride]) * complection->strength;
    temp1 += (temp2 - temp1) * complection->dexterity;
    return value += (temp1 - value) * complection->height;
}

typedef struct {
    uint32_t type;
    size_t count;
    ce_figfile_value_callback callback;
} ce_figfile_value_tuple;

static const ce_figfile_value_tuple ce_figfile_value_tuples[] = {
    { 0x31474946, 1, ce_figfile_value_fig1 },
    { 0x38474946, 8, ce_figfile_value_fig8 }
};

static const
ce_figfile_value_tuple* ce_figfile_value_tuple_choose(uint32_t type)
{
    for (size_t i = 0; i < sizeof(ce_figfile_value_tuples) /
                            sizeof(ce_figfile_value_tuples[0]); ++i) {
        if (ce_figfile_value_tuples[i].type == type) {
            return &ce_figfile_value_tuples[i];
        }
    }
    return NULL;
}

ce_figfile* ce_figfile_open(ce_res_file* res_file, const char* name)
{
    size_t index = ce_res_file_node_index(res_file, name);

    ce_figfile* figfile = ce_alloc(sizeof(ce_figfile));
    figfile->size = ce_res_file_node_size(res_file, index);
    figfile->data = ce_res_file_node_data(res_file, index);

    union {
        float* f;
        uint16_t* u16;
        uint32_t* u32;
    } ptr = {figfile->data};

    const ce_figfile_value_tuple* value_tuple =
        ce_figfile_value_tuple_choose(ce_le2cpu32(*ptr.u32++));
    assert(NULL != value_tuple && "wrong signature");

    figfile->value_count = value_tuple->count;
    figfile->value_callback = value_tuple->callback;
    figfile->vertex_count = ce_le2cpu32(*ptr.u32++);
    figfile->normal_count = ce_le2cpu32(*ptr.u32++);
    figfile->texcoord_count = ce_le2cpu32(*ptr.u32++);
    figfile->index_count = ce_le2cpu32(*ptr.u32++);
    figfile->vertex_component_count = ce_le2cpu32(*ptr.u32++);
    figfile->morph_component_count = ce_le2cpu32(*ptr.u32++);
    figfile->user_data_offset = ce_le2cpu32(*ptr.u32++);
    figfile->material_group = ce_le2cpu32(*ptr.u32++);
    figfile->texture_number = ce_le2cpu32(*ptr.u32++);

    figfile->center = ptr.f;
    figfile->min = ptr.f += 3 * figfile->value_count;
    figfile->max = ptr.f += 3 * figfile->value_count;
    figfile->radius = ptr.f += 3 * figfile->value_count;

    figfile->vertices = ptr.f += figfile->value_count;
    figfile->normals = ptr.f += 3 * figfile->value_count *
                                4 * figfile->vertex_count;
    figfile->texcoords = ptr.f += 4 * 4 * figfile->normal_count;
    figfile->indices = (ptr.f += 2 * figfile->texcoord_count, ptr.u16);
    figfile->vertex_components = ptr.u16 += figfile->index_count;
    figfile->morph_components = ptr.u16 += 3 * figfile->vertex_component_count;

    for (int i = 0; i < figfile->index_count; ++i) {
        ce_le2cpu16s(&figfile->indices[i]);
    }

    for (int i = 0; i < 3 * figfile->vertex_component_count; ++i) {
        ce_le2cpu16s(&figfile->vertex_components[i]);
    }

    for (int i = 0; i < 2 * figfile->morph_component_count; ++i) {
        ce_le2cpu16s(&figfile->morph_components[i]);
    }

    return figfile;
}

void ce_figfile_close(ce_figfile* figfile)
{
    if (NULL != figfile) {
        ce_free(figfile->data, figfile->size);
        ce_free(figfile, sizeof(ce_figfile));
    }
}
