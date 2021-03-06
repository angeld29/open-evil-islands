/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_RESFILE_HPP
#define CE_RESFILE_HPP

#include <cstdint>
#include <ctime>

#include <boost/filesystem/path.hpp>

#include "string.hpp"
#include "memfile.hpp"

namespace cursedearth
{
    typedef struct {
        ce_string* name;
        int32_t next_index;
        uint32_t data_length;
        uint32_t data_offset;
        int32_t modified;
        uint16_t name_length;
        uint32_t name_offset;
    } ce_res_node;

    /**
     * @brief doc/formats/res.txt
     */
    typedef struct {
        ce_string* name;
        uint32_t node_count;
        uint32_t metadata_offset;
        uint32_t names_length;
        char* names;
        ce_res_node* nodes;
        ce_mem_file* mem_file;
    } ce_res_file;

    // res file takes ownership of the mem file if successfull
    ce_res_file* ce_res_file_new(const std::string& name, ce_mem_file*);
    ce_res_file* ce_res_file_new_path(const boost::filesystem::path&);
    void ce_res_file_del(ce_res_file* res_file);

    size_t ce_res_file_node_index(const ce_res_file* res_file, const std::string& name);

    inline const char* ce_res_file_node_name(const ce_res_file* res_file, size_t index)
    {
        return res_file->nodes[index].name->str;
    }

    inline size_t ce_res_file_node_size(const ce_res_file* res_file, size_t index)
    {
        return res_file->nodes[index].data_length;
    }

    inline time_t ce_res_file_node_modified(const ce_res_file* res_file, int index)
    {
        return res_file->nodes[index].modified;
    }

    void* ce_res_file_node_data(ce_res_file* res_file, size_t index);
}

#endif
