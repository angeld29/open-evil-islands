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
#include <vector>
#include <algorithm>
#include <exception>

#include "alloc.hpp"
#include "logging.hpp"
#include "memfile.hpp"
#include "mobfile.hpp"

#define CE_MOB_UNIT_CAST(MF) \
mob_unit* unit = static_cast<mob_unit*>((MF)->objects->back());

#define CE_MOB_UNIT_LOGIC_CAST(MF) \
CE_MOB_UNIT_CAST(MF); \
mob_unit_logic* unit_logic = unit->logics->back();

#define CE_MOB_LEVER_CAST(MF) \
mob_lever* lever = static_cast<mob_lever*>((MF)->objects->back());

#define CE_MOB_TRAP_CAST(MF) \
mob_trap* trap = static_cast<mob_trap*>((MF)->objects->back());

#define CE_MOB_READ_VECTOR_OF_STRINGS(V) \
if (!(V)) { \
    (V) = new std::vector<ce_string*>(); \
    (V)->reserve(ce_mem_file_read_u32le(mem_file)); \
    block_loop(mem_file, size -= 4); \
} else { \
    (V)->push_back(read_string(mem_file, size)); \
}

namespace cursedearth
{

    void delete_ce_string_vector(std::vector<ce_string*>* vector){
        for(auto& str : *vector) {
           ce_string_del(str);
        }
        delete vector;
    }

    mob_object::~mob_object()
    {
        for(auto& str : *parts) {
           ce_string_del(str);
        }
        ce_string_del(quest_info);
        ce_string_del(comment);
        ce_string_del(secondary_texture);
        ce_string_del(primary_texture);
        ce_string_del(parent_name);
        ce_string_del(model_name);
        ce_string_del(name);
        delete parts;
    }

    mob_unit::~mob_unit()
    {
        for(auto& logic : *logics) {
           delete logic;
        }
        delete logics;
        delete_ce_string_vector(quest_items);
        delete_ce_string_vector(quick_items);
        delete_ce_string_vector(spells);
        delete_ce_string_vector(weapons);
        delete_ce_string_vector(armors);
        ce_string_del(name);
    }

    mob_trap::~mob_trap()
    {
        if (target) {
            delete target;
        }
        if (area) {
            delete area;
        }
        ce_string_del(spell);
    }

    mob_file::mob_file(const boost::filesystem::path& path)
    {
        ce_mem_file* mem_file = ce_mem_file_new_path(path);
        if (NULL == mem_file) {
            throw std::invalid_argument("Could not open mob file.");
        }

        name = ce_string_new_str(path.filename().string().c_str());
        block_loop(mem_file, ce_mem_file_size(mem_file));

        ce_mem_file_del(mem_file);
    }

    mob_file::~mob_file()
    {
        if (objects) {
            for(auto& object : *objects) {
               delete object;
            }
            delete objects;
        }
        ce_string_del(script);
        ce_string_del(name);
    }

    void mob_file::decrypt_script(char* data, size_t size, uint32_t key)
    {
        for (size_t i = 0; i < size; ++i) {
            key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
            data[i] ^= key >> 16;
        }
    }


    ce_string* mob_file::get_name()
    {
        return name;
    }

    ce_string* mob_file::get_script()
    {
        return script;
    }

    std::vector<mob_object*>* mob_file::get_objects()
    {
        return objects;
    }

    ce_string* mob_file::read_string(ce_mem_file* mem_file, size_t size)
    {
        std::vector<char> buffer(size);
        ce_mem_file_read(mem_file, buffer.data(), 1, size);
        ce_string* st = ce_string_new_str_n(buffer.data(), size);
//        if(st) {
//            ce_logging_info("read string: %s", st->str);
//        } else {
//            ce_logging_info("could not read string, apparently");
//        }
        return st;
    }

    // callbacks

    // void mob_file::mob_file_block_loop(ce_mem_file*, size_t);

    void mob_file::block_unknown(ce_mem_file* mem_file, size_t size)
    {
        ce_mem_file_skip(mem_file, size);
    }

    void mob_file::block_main(ce_mem_file* mem_file, size_t size)
    {
        block_loop(mem_file, size);
    }

    void mob_file::block_quest(ce_mem_file* mem_file, size_t size)
    {
        block_loop(mem_file, size);
    }

    void mob_file::block_zonal(ce_mem_file* mem_file, size_t size)
    {
        block_loop(mem_file, size);
    }

    void mob_file::block_text(ce_mem_file* mem_file, size_t size)
    {
        // WARNING: string is encrypted and may contain null characters
        uint32_t key = ce_mem_file_read_u32le(mem_file);
        std::vector<char> buffer(size -= 4);
        ce_mem_file_read(mem_file, buffer.data(), 1, size);
        decrypt_script(buffer.data(), size, key);
        script = ce_string_new_str_n(buffer.data(), size);
    }

    void mob_file::block_object(ce_mem_file* mem_file, size_t size)
    {
        objects = new std::vector<mob_object*>();
        block_loop(mem_file, size);
    }

    // object

    void mob_file::block_object_object(ce_mem_file* mem_file, size_t size)
    {
        objects->push_back(new mob_object());
        block_loop(mem_file, size);
    }

    void mob_file::block_object_object_parts(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        CE_MOB_READ_VECTOR_OF_STRINGS(object->parts);
    }

    void mob_file::block_object_object_owner(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        object->owner = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_object_id(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        object->id = ce_mem_file_read_u32le(mem_file);
    }

    void mob_file::block_object_object_type(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        object->type = ce_mem_file_read_u32le(mem_file);
    }

    void mob_file::block_object_object_name(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->name = read_string(mem_file, size);
    }

    void mob_file::block_object_object_model_name(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->model_name = read_string(mem_file, size);
    }

    void mob_file::block_object_object_parent_name(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->parent_name = read_string(mem_file, size);
    }

    void mob_file::block_object_object_primary_texture(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->primary_texture = read_string(mem_file, size);
    }

    void mob_file::block_object_object_secondary_texture(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->secondary_texture = read_string(mem_file, size);
    }

    void mob_file::block_object_object_comment(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->comment = read_string(mem_file, size);
    }

    void mob_file::block_object_object_position(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        ce_mem_file_read(mem_file, object->position, sizeof(float), 3);
    }

    void mob_file::block_object_object_rotation(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        ce_mem_file_read(mem_file, object->rotation, sizeof(float), 4);
    }

    void mob_file::block_object_object_quest(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        object->quest = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_object_shadow(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        object->shadow = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_object_parent_id(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        object->parent_id = ce_mem_file_read_u32le(mem_file);
    }

    void mob_file::block_object_object_quest_info(ce_mem_file* mem_file, size_t size)
    {
        mob_object* object = objects->back();
        object->quest_info = read_string(mem_file, size);
    }

    void mob_file::block_object_object_complection(ce_mem_file* mem_file, size_t)
    {
        mob_object* object = objects->back();
        ce_mem_file_read(mem_file, object->complection, sizeof(float), 3);
    }

    // unit

    void mob_file::block_object_unit(ce_mem_file* mem_file, size_t size)
    {
        objects->push_back(new mob_unit());
        block_loop(mem_file, size);
    }

    void mob_file::block_object_unit_need_import(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_CAST(this);
        unit->need_import = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_unit_name(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        unit->name = read_string(mem_file, size);
    }

    void mob_file::block_object_unit_armors(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        CE_MOB_READ_VECTOR_OF_STRINGS(unit->armors);
    }

    void mob_file::block_object_unit_weapons(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        CE_MOB_READ_VECTOR_OF_STRINGS(unit->weapons);
    }

    void mob_file::block_object_unit_spells(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        CE_MOB_READ_VECTOR_OF_STRINGS(unit->spells);
    }

    void mob_file::block_object_unit_quick_items(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        CE_MOB_READ_VECTOR_OF_STRINGS(unit->quick_items);
    }

    void mob_file::block_object_unit_quest_items(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        CE_MOB_READ_VECTOR_OF_STRINGS(unit->quest_items);
    }

    void mob_file::block_object_unit_stats(ce_mem_file* mem_file, size_t size)
    {
        block_unknown(mem_file, size);
    }

    // unit logic

    void mob_file::block_object_unit_logic(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(this);
        if (!unit->logics) {
            unit->logics = new std::vector<mob_unit_logic*>(8);
        }
        unit->logics->push_back(new mob_unit_logic());
        block_loop(mem_file, size);
    }

    void mob_file::block_object_unit_logic_alarm_condition(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->alarm_condition = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_unit_logic_help(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->help = ce_mem_file_read_fle(mem_file);
    }

    void mob_file::block_object_unit_logic_cyclic(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->cyclic = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_unit_logic_aggression_mode(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->aggression_mode = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_unit_logic_always_active(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->always_active = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_unit_logic_model(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->model = ce_mem_file_read_u32le(mem_file);
    }

    void mob_file::block_object_unit_logic_guard_radius(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->guard_radius = ce_mem_file_read_fle(mem_file);
    }

    void mob_file::block_object_unit_logic_wait(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->wait = ce_mem_file_read_fle(mem_file);
    }

    void mob_file::block_object_unit_logic_guard_position(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        ce_mem_file_read(mem_file, unit_logic->guard_position, sizeof(float), 3);
    }

    void mob_file::block_object_unit_logic_use(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->use = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_unit_logic_nalarm(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(this);
        unit_logic->nalarm = ce_mem_file_read_u8(mem_file);
    }

    // lever

    void mob_file::block_object_lever(ce_mem_file* mem_file, size_t size)
    {
        objects->push_back(new mob_lever());
        block_loop(mem_file, size);
    }

    void mob_file::block_object_lever_stats(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(this);
        ce_mem_file_read(mem_file, lever->stats, sizeof(uint32_t), 3);
    }

    void mob_file::block_object_lever_state(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(this);
        lever->state = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_lever_state_count(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(this);
        lever->state_count = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_lever_cyclic(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(this);
        lever->cyclic = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_lever_door(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(this);
        lever->door = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_lever_recalc_graph(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(this);
        lever->recalc_graph = ce_mem_file_read_u8(mem_file);
    }

    // trap

    void mob_file::block_object_trap(ce_mem_file* mem_file, size_t size)
    {
        objects->push_back(new mob_trap());
        block_loop(mem_file, size);
    }

    void mob_file::block_object_trap_diplomacy(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_TRAP_CAST(this);
        trap->diplomacy = ce_mem_file_read_u32le(mem_file);
    }

    void mob_file::block_object_trap_spell(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_TRAP_CAST(this);
        trap->spell = read_string(mem_file, size);
    }

    void mob_file::block_object_trap_cast_interval(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_TRAP_CAST(this);
        trap->cast_interval = ce_mem_file_read_u32le(mem_file);
    }

    void mob_file::block_object_trap_cast_once(ce_mem_file* mem_file, size_t)
    {
        CE_MOB_TRAP_CAST(this);
        trap->cast_once = ce_mem_file_read_u8(mem_file);
    }

    void mob_file::block_object_trap_area(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_TRAP_CAST(this);
        uint32_t count = ce_mem_file_read_u32le(mem_file);
        size -= 4;
        trap->area = new mob_trap_area();
        trap->area->size = size;
        trap->area->count = count;
        ce_mem_file_read(mem_file, trap->area->values, 1, size);
    }

    void mob_file::block_object_trap_target(ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_TRAP_CAST(this);
        uint32_t count = ce_mem_file_read_u32le(mem_file);
        size -= 4;
        trap->target = new mob_trap_target();
        trap->target->size = size;
        trap->target->count = count;
        ce_mem_file_read(mem_file, trap->target->values, 1, size);
    }

    void mob_file::block_loop(ce_mem_file* mem_file, size_t size)
    {
        while (0 != size) {
            uint32_t child_type = ce_mem_file_read_u32le(mem_file);
            uint32_t child_size = ce_mem_file_read_u32le(mem_file);

            size -= child_size;
            child_size -= 4 + 4;

    #if 0
            // WARNING: graph data reversing
            if (0x31415926 == child_type) {
                int sz = child_size;
                uint32_t s1, s2;
                int16_t v;
                ce_mem_file_read(mem_file, &s1, sizeof(s1), 1);
                ce_mem_file_read(mem_file, &s2, sizeof(s2), 1);
                printf("%u %u\n", s1 / 8u, s2 / 8u);
                printf("%u %u\n", s1, s2);
                sz -= sizeof(s1) + sizeof(s2);
                printf("%d\n\n", sz);

                // 1: s1/8 matrices 8 x 8
                printf("---1---\n\n");
                for (unsigned int i = 0; i < s1 / 8; ++i) {
                    for (unsigned int j = 0; j < 8; ++j) {
                        for (unsigned int k = 0; k < 8; ++k) {
                            ce_mem_file_read(mem_file, &v, sizeof(v), 1);
                            sz -= sizeof(v);
                            printf("%hd ", v);
                        }
                        printf("\n");
                    }
                    printf("\n");
                }
                printf("\n");

                // zonemenu 2x2   38912   sz, 127 loops, 304  b per loop, 48  b tail
                // bz8k     4x4   155648  sz, 255 loops, 608  b per loop, 96  b tail
                // zone2    4x6   233472  sz, 383 loops, 608  b per loop, 96  b tail
                // zone20   5x5   243200  sz, 319 loops, 760  b per loop, 120 b tail
                // zone6lmp 7x12  817152  sz, 767 loops, 1064 b per loop, 168 b tail
                // zone1    8x6   466944  sz, 383 loops, 1216 b per loop, 192 b tail
                // zone8    11x11 1177088 sz, 703 loops, 1672 b per loop, 264 b tail
                // zone7    15x10 1459200 sz, 639 loops, 2280 b per loop, 360 b tail
                // zone11   16x12 1867776 sz, 767 loops, 2432 b per loop, 384 b tail

                // 2: in loop
                for (unsigned int i = 0; i < 127; ++i) {
                    printf("---2.1 loop %u---%d\n\n", i + 1, sz);
                    // 2.1: 4*s1/8 numbers
                    for (unsigned int j = 0; j < 4 * s1 / 8; ++j) {
                        ce_mem_file_read(mem_file, &v, sizeof(v), 1);
                        sz -= sizeof(v);
                        printf("%hd ", v);
                        if (0 == (j + 1) % 8) printf("\n");
                    }

                    // 2.2: 1 matrix s1/8 x 8
                    printf("\n\n---2.2 loop %u---\n\n", i + 1);
                    for (unsigned int j = 0; j < s1 / 8; ++j) {
                        for (unsigned int k = 0; k < 8; ++k) {
                            ce_mem_file_read(mem_file, &v, sizeof(v), 1);
                            sz -= sizeof(v);
                            printf("%hd ", v);
                        }
                        printf("\n");
                    }

                    // 2.3: 1 matrix s1 x 8
                    printf("\n---2.3 loop %u---\n\n", i + 1);
                    for (unsigned int j = 0; j < s1; ++j) {
                        for (unsigned int k = 0; k < 8; ++k) {
                            ce_mem_file_read(mem_file, &v, sizeof(v), 1);
                            sz -= sizeof(v);
                            printf("%hd ", v);
                        }
                        printf("\n");
                    }
                    printf("\n");
                }
                printf("---3---\n\n");

                // 3: s1/8 x 12 numbers
                for (unsigned int i = 0; i < s1 / 8; ++i) {
                    for (unsigned int j = 0; j < 12; ++j) {
                        ce_mem_file_read(mem_file, &v, sizeof(v), 1);
                        sz -= sizeof(v);
                        printf("%hd ", v);
                    }
                    printf("\n");
                }

                printf("\n%d\n", sz);
            }
    #endif
            (this->*choose_callback(child_type))(mem_file, child_size);
        }
    }

    mob_file::block_callback mob_file::choose_callback(uint32_t type)
    {
        for (size_t i = 0; i < sizeof(block_pairs) / sizeof(block_pairs[0]); ++i) {
            if (block_pairs[i].type == type) {
                return block_pairs[i].callback;
            }
        }
        return &mob_file::block_unknown;
    }

}
