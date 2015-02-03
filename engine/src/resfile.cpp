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

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cassert>

#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "byteorder.hpp"
#include "resfile.hpp"

static const uint32_t CE_RES_SIGNATURE = 0x19ce23c;

ce_res_file* ce_res_file_new(const char* name, ce_mem_file* mem_file)
{
    ce_res_file* res_file = ce_alloc_zero(sizeof(ce_res_file));
    res_file->name = ce_string_new_str(name);
    res_file->mem_file = mem_file;

    uint32_t CE_UNUSED(signature) = ce_mem_file_read_u32le(mem_file);
    assert(CE_RES_SIGNATURE == signature && "wrong signature");

    res_file->node_count = ce_mem_file_read_u32le(mem_file);
    res_file->metadata_offset = ce_mem_file_read_u32le(mem_file);
    res_file->names_length = ce_mem_file_read_u32le(mem_file);
    res_file->nodes = ce_alloc_zero(sizeof(ce_res_node) * res_file->node_count);

    ce_mem_file_seek(mem_file, res_file->metadata_offset, CE_MEM_FILE_SEEK_SET);

    for (size_t i = 0; i < res_file->node_count; ++i) {
        res_file->nodes[i].next_index = ce_mem_file_read_i32le(mem_file);
        res_file->nodes[i].data_length = ce_mem_file_read_u32le(mem_file);
        res_file->nodes[i].data_offset = ce_mem_file_read_u32le(mem_file);
        res_file->nodes[i].modified = ce_mem_file_read_i32le(mem_file);
        res_file->nodes[i].name_length = ce_mem_file_read_u16le(mem_file);
        res_file->nodes[i].name_offset = ce_mem_file_read_u32le(mem_file);
    }

    res_file->names = ce_alloc(res_file->names_length);
    ce_mem_file_read(mem_file, res_file->names, 1, res_file->names_length);

    for (size_t i = 0; i < res_file->node_count; ++i) {
        res_file->nodes[i].name = ce_string_new_str_n(res_file->names +
            res_file->nodes[i].name_offset, res_file->nodes[i].name_length);
    }

    return res_file;
}

ce_res_file* ce_res_file_new_path(const char* path)
{
    ce_mem_file* mem_file = ce_mem_file_new_path(path);
    if (NULL == mem_file) {
        return NULL;
    }

    const char* name = ce_strrpbrk(path, "\\/");
    if (NULL == name) {
        name = path;
    } else {
        ++name;
    }

    return ce_res_file_new(name, mem_file);
}

void ce_res_file_del(ce_res_file* res_file)
{
    if (NULL != res_file) {
        ce_mem_file_del(res_file->mem_file);
        if (NULL != res_file->nodes) {
            for (size_t i = 0; i < res_file->node_count; ++i) {
                ce_string_del(res_file->nodes[i].name);
            }
            ce_free(res_file->nodes, sizeof(ce_res_node) * res_file->node_count);
        }
        ce_free(res_file->names, res_file->names_length);
        ce_string_del(res_file->name);
        ce_free(res_file, sizeof(ce_res_file));
    }
}

inline int ce_res_name_hash(const char* name, int limit)
{
    int sum = 0;
    while (*name) {
        sum += tolower(*name++);
    }
    return sum % limit;
}

size_t ce_res_file_node_index(const ce_res_file* res_file, const char* name)
{
    int index = ce_res_name_hash(name, res_file->node_count);
    for (const ce_res_node* node; index >= 0; index = node->next_index) {
        node = res_file->nodes + index;
        if (0 == ce_strcasecmp(name, node->name->str)) {
            break;
        }
    }
    return -1 != index ? (size_t)index : res_file->node_count;
}

void* ce_res_file_node_data(ce_res_file* res_file, size_t index)
{
    void* data = ce_alloc(res_file->nodes[index].data_length);
    ce_mem_file_seek(res_file->mem_file, res_file->nodes[index].data_offset, CE_MEM_FILE_SEEK_SET);
    ce_mem_file_read(res_file->mem_file, data, 1, res_file->nodes[index].data_length);
    return data;
}
