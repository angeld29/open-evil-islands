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
#include <vector>

#include "alloc.hpp"
#include "logging.hpp"
#include "regfile.hpp"

namespace cursedearth
{
    const uint32_t CE_REG_SIGNATURE = 0x45ab3efbu;

    ce_property* ce_reg_create_option_int(ce_mem_file* mem_file, const char* name)
    {
        ce_property* property = ce_property_new(name, CE_TYPE_INT);
        ce_value_set_int(property->value, ce_mem_file_read_i32le(mem_file));
        return property;
    }

    ce_property* ce_reg_create_option_float(ce_mem_file* mem_file, const char* name)
    {
        ce_property* property = ce_property_new(name, CE_TYPE_FLOAT);
        ce_value_set_float(property->value, ce_mem_file_read_fle(mem_file));
        return property;
    }

    ce_property* ce_reg_create_option_string(ce_mem_file* mem_file, const char* name)
    {
        size_t length = ce_mem_file_read_u16le(mem_file);

        std::vector<char> value(length + 1);
        value[length] = '\0';

        ce_mem_file_read(mem_file, value.data(), 1, length);

        ce_property* property = ce_property_new(name, CE_TYPE_STRING);
        ce_value_set_string(property->value, value.data());

        return property;
    }

    ce_property* (*ce_reg_create_option_procs[])(ce_mem_file*, const char*) = {
        ce_reg_create_option_int,
        ce_reg_create_option_float,
        ce_reg_create_option_string,
    };

    ce_reg_file* ce_reg_file_new(ce_mem_file* mem_file)
    {
        uint32_t signature = ce_mem_file_read_u32le(mem_file);
        assert(CE_REG_SIGNATURE == signature && "wrong signature");

        uint16_t section_count = ce_mem_file_read_u16le(mem_file);

        ce_reg_file* reg_file = (ce_reg_file*)ce_alloc_zero(sizeof(ce_reg_file));
        reg_file->sections = ce_vector_new_reserved(section_count);

        struct section_t
        {
            uint32_t offset;
            uint16_t option_count;
            uint16_t name_length;
        };

        std::vector<section_t> sections(section_count);
        for (size_t i = 0; i < section_count; ++i) {
            ce_mem_file_skip(mem_file, 2);
            sections[i].offset = ce_mem_file_read_u32le(mem_file);
        }

        for (size_t i = 0; i < section_count; ++i) {
            ce_mem_file_seek(mem_file, sections[i].offset, CE_MEM_FILE_SEEK_SET);

            sections[i].option_count = ce_mem_file_read_u16le(mem_file);
            sections[i].name_length = ce_mem_file_read_u16le(mem_file);

            std::vector<char> section_name(sections[i].name_length + 1);
            section_name[sections[i].name_length] = '\0';

            ce_mem_file_read(mem_file, section_name.data(), 1, sections[i].name_length);

            ce_object* section = ce_object_new(section_name.data());
            ce_vector_push_back(reg_file->sections, section);

            struct option_t
            {
                uint32_t offset;
                uint8_t type;
                uint16_t name_length;
            };

            std::vector<option_t> options(sections[i].option_count);
            for (size_t j = 0; j < sections[i].option_count; ++j) {
                ce_mem_file_skip(mem_file, 2);
                options[j].offset = ce_mem_file_read_u32le(mem_file);
            }

            for (size_t j = 0; j < sections[i].option_count; ++j) {
                ce_mem_file_seek(mem_file, sections[i].offset + options[j].offset, CE_MEM_FILE_SEEK_SET);

                options[j].type = ce_mem_file_read_u8(mem_file);
                options[j].name_length = ce_mem_file_read_u16le(mem_file);

                std::vector<char> option_name(options[j].name_length + 1);
                option_name[options[j].name_length] = '\0';

                ce_mem_file_read(mem_file, option_name.data(), 1, options[j].name_length);
                uint16_t value_count = 1;

                if (options[j].type >= 128) {
                    options[j].type -= 128;
                    value_count = ce_mem_file_read_u16le(mem_file);
                }

                for (uint16_t k = 0; k < value_count; ++k) {
                    std::vector<char> array_name(options[j].name_length + 8);
                    snprintf(array_name.data(), array_name.size(), "%s%hu", option_name.data(), k);
                    ce_object_add(section, (*ce_reg_create_option_procs[options[j].type])(mem_file, array_name.data()));
                }
            }
        }

        return reg_file;
    }

    void ce_reg_file_del(ce_reg_file* reg_file)
    {
        if (NULL != reg_file) {
            ce_vector_for_each(reg_file->sections, (void(*)(void*))ce_object_del);
            ce_vector_del(reg_file->sections);
            ce_free(reg_file, sizeof(ce_reg_file));
        }
    }

    ce_value* ce_reg_file_find(ce_reg_file* reg_file, const char* section_name, const char* option_name, size_t index)
    {
        std::vector<char> array_name(strlen(option_name) + 8);
        snprintf(array_name.data(), array_name.size(), "%s%zu", option_name, index);

        for (size_t i = 0; i < reg_file->sections->count; ++i) {
            ce_object* section = (ce_object*)reg_file->sections->items[i];
            if (0 == strcmp(section_name, section->name->str)) {
                ce_property* option = ce_object_find(section, array_name.data());
                if (NULL != option) {
                    return option->value;
                }
            }
        }
        return NULL;
    }
}
