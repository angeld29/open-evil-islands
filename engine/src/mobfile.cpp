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

#include "lib.hpp"
#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "memfile.hpp"
#include "mobfile.hpp"

#define CE_MOB_UNIT_CAST(MF) \
ce_mob_object* mob_object = (ce_mob_object*)ce_vector_back((MF)->objects); \
ce_mob_unit* mob_unit = (ce_mob_unit*)mob_object->impl;

#define CE_MOB_UNIT_LOGIC_CAST(MF) \
CE_MOB_UNIT_CAST(MF); \
ce_mob_unit_logic* mob_unit_logic = (ce_mob_unit_logic*)ce_vector_back(mob_unit->logics);

#define CE_MOB_LEVER_CAST(MF) \
ce_mob_object* mob_object = (ce_mob_object*)ce_vector_back((MF)->objects); \
ce_mob_lever* mob_lever = (ce_mob_lever*)mob_object->impl;

#define CE_MOB_TRAP_CAST(MF) \
ce_mob_object* mob_object = (ce_mob_object*)ce_vector_back((MF)->objects); \
ce_mob_trap* mob_trap = (ce_mob_trap*)mob_object->impl;

#define CE_MOB_READ_VECTOR_OF_STRINGS(V) \
if (NULL == (V)) { \
    (V) = ce_vector_new_reserved(ce_mem_file_read_u32le(mem_file)); \
    ce_mob_file_block_loop(mob_file, mem_file, size -= 4); \
} else { \
    ce_vector_push_back((V), ce_mob_read_string(mem_file, size)); \
}

namespace cursedearth
{
    ce_mob_object* ce_mob_object_new(ce_mob_object_vtable vtable)
    {
        ce_mob_object* mob_object = (ce_mob_object*)ce_alloc_zero(sizeof(ce_mob_object));
        mob_object->vtable = vtable;
        mob_object->impl = ce_alloc_zero(vtable.size);
        return mob_object;
    }

    void ce_mob_object_del(ce_mob_object* mob_object)
    {
        if (NULL != mob_object) {
            if (NULL != mob_object->vtable.dtor) {
                (*mob_object->vtable.dtor)(mob_object);
            }
            ce_vector_for_each(mob_object->parts, (void(*)(void*))ce_string_del);
            ce_string_del(mob_object->quest_info);
            ce_string_del(mob_object->comment);
            ce_string_del(mob_object->secondary_texture);
            ce_string_del(mob_object->primary_texture);
            ce_string_del(mob_object->parent_name);
            ce_string_del(mob_object->model_name);
            ce_string_del(mob_object->name);
            ce_vector_del(mob_object->parts);
            ce_free(mob_object->impl, mob_object->vtable.size);
            ce_free(mob_object, sizeof(ce_mob_object));
        }
    }

    ce_mob_unit_logic* ce_mob_unit_logic_new(void)
    {
        return (ce_mob_unit_logic*)ce_alloc_zero(sizeof(ce_mob_unit_logic));
    }

    void ce_mob_unit_logic_del(ce_mob_unit_logic* mob_unit_logic)
    {
        ce_free(mob_unit_logic, sizeof(ce_mob_unit_logic));
    }

    void ce_mob_unit_dtor(ce_mob_object* mob_object)
    {
        ce_mob_unit* mob_unit = (ce_mob_unit*)mob_object->impl;
        ce_vector_for_each(mob_unit->logics, (void(*)(void*))ce_mob_unit_logic_del);
        ce_vector_for_each(mob_unit->quest_items, (void(*)(void*))ce_string_del);
        ce_vector_for_each(mob_unit->quick_items, (void(*)(void*))ce_string_del);
        ce_vector_for_each(mob_unit->spells, (void(*)(void*))ce_string_del);
        ce_vector_for_each(mob_unit->weapons, (void(*)(void*))ce_string_del);
        ce_vector_for_each(mob_unit->armors, (void(*)(void*))ce_string_del);
        ce_vector_del(mob_unit->logics);
        ce_vector_del(mob_unit->quest_items);
        ce_vector_del(mob_unit->quick_items);
        ce_vector_del(mob_unit->spells);
        ce_vector_del(mob_unit->weapons);
        ce_vector_del(mob_unit->armors);
        ce_string_del(mob_unit->name);
    }

    void ce_mob_trap_dtor(ce_mob_object* mob_object)
    {
        ce_mob_trap* mob_trap = (ce_mob_trap*)mob_object->impl;
        if (NULL != mob_trap->target) {
            ce_free(mob_trap->target, sizeof(ce_mob_trap_target) + mob_trap->target->size);
        }
        if (NULL != mob_trap->area) {
            ce_free(mob_trap->area, sizeof(ce_mob_trap_area) + mob_trap->area->size);
        }
        ce_string_del(mob_trap->spell);
    }

    void ce_mob_decrypt_script(char* data, size_t size, uint32_t key)
    {
        for (size_t i = 0; i < size; ++i) {
            key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
            data[i] ^= key >> 16;
        }
    }

    ce_string* ce_mob_read_string(ce_mem_file* mem_file, size_t size)
    {
        std::vector<char> buffer(size);
        ce_mem_file_read(mem_file, buffer.data(), 1, size);
        return ce_string_new_str_n(buffer.data(), size);
    }

    // callbacks

    void ce_mob_file_block_loop(ce_mob_file*, ce_mem_file*, size_t);

    void ce_mob_file_block_unknown(ce_mob_file*, ce_mem_file* mem_file, size_t size)
    {
        ce_mem_file_skip(mem_file, size);
    }

    void ce_mob_file_block_main(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_quest(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_zonal(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_text(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        // WARNING: string is encrypted and may contain null characters
        uint32_t key = ce_mem_file_read_u32le(mem_file);
        std::vector<char> buffer(size -= 4);
        ce_mem_file_read(mem_file, buffer.data(), 1, size);
        ce_mob_decrypt_script(buffer.data(), size, key);
        mob_file->script = ce_string_new_str_n(buffer.data(), size);
    }

    void ce_mob_file_block_object(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        mob_file->objects = ce_vector_new();
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    // object

    void ce_mob_file_block_object_object(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object_vtable vt = { 0, NULL };
        ce_vector_push_back(mob_file->objects, ce_mob_object_new(vt));
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_object_object_parts(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        CE_MOB_READ_VECTOR_OF_STRINGS(object->parts);
    }

    void ce_mob_file_block_object_object_owner(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->owner = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_object_id(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->id = ce_mem_file_read_u32le(mem_file);
    }

    void ce_mob_file_block_object_object_type(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->type = ce_mem_file_read_u32le(mem_file);
    }

    void ce_mob_file_block_object_object_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->name = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_model_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->model_name = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_parent_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->parent_name = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_primary_texture(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->primary_texture = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_secondary_texture(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->secondary_texture = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_comment(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->comment = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_position(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        ce_mem_file_read(mem_file, object->position, sizeof(float), 3);
    }

    void ce_mob_file_block_object_object_rotation(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        ce_mem_file_read(mem_file, object->rotation, sizeof(float), 4);
    }

    void ce_mob_file_block_object_object_quest(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->quest = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_object_shadow(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->shadow = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_object_parent_id(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->parent_id = ce_mem_file_read_u32le(mem_file);
    }

    void ce_mob_file_block_object_object_quest_info(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        object->quest_info = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_object_complection(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        ce_mob_object* object = (ce_mob_object*)ce_vector_back(mob_file->objects);
        ce_mem_file_read(mem_file, object->complection, sizeof(float), 3);
    }

    // unit

    void ce_mob_file_block_object_unit(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object_vtable vt = { sizeof(ce_mob_unit), ce_mob_unit_dtor };
        ce_vector_push_back(mob_file->objects, ce_mob_object_new(vt));
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_object_unit_need_import(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_CAST(mob_file);
        mob_unit->need_import = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_unit_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        mob_unit->name = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_unit_armors(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        CE_MOB_READ_VECTOR_OF_STRINGS(mob_unit->armors);
    }

    void ce_mob_file_block_object_unit_weapons(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        CE_MOB_READ_VECTOR_OF_STRINGS(mob_unit->weapons);
    }

    void ce_mob_file_block_object_unit_spells(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        CE_MOB_READ_VECTOR_OF_STRINGS(mob_unit->spells);
    }

    void ce_mob_file_block_object_unit_quick_items(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        CE_MOB_READ_VECTOR_OF_STRINGS(mob_unit->quick_items);
    }

    void ce_mob_file_block_object_unit_quest_items(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        CE_MOB_READ_VECTOR_OF_STRINGS(mob_unit->quest_items);
    }

    void ce_mob_file_block_object_unit_stats(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_file_block_unknown(mob_file, mem_file, size);
    }

    // unit logic

    void ce_mob_file_block_object_unit_logic(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_UNIT_CAST(mob_file);
        if (NULL == mob_unit->logics) {
            mob_unit->logics = ce_vector_new_reserved(8);
        }
        ce_vector_push_back(mob_unit->logics, ce_mob_unit_logic_new());
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_object_unit_logic_alarm_condition(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->alarm_condition = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_help(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->help = ce_mem_file_read_fle(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_cyclic(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->cyclic = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_aggression_mode(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->aggression_mode = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_always_active(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->always_active = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_model(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->model = ce_mem_file_read_u32le(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_guard_radius(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->guard_radius = ce_mem_file_read_fle(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_wait(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->wait = ce_mem_file_read_fle(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_guard_position(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        ce_mem_file_read(mem_file, mob_unit_logic->guard_position, sizeof(float), 3);
    }

    void ce_mob_file_block_object_unit_logic_use(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->use = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_unit_logic_nalarm(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_UNIT_LOGIC_CAST(mob_file);
        mob_unit_logic->nalarm = ce_mem_file_read_u8(mem_file);
    }

    // lever

    void ce_mob_file_block_object_lever(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object_vtable vt = { sizeof(ce_mob_lever), NULL };
        ce_vector_push_back(mob_file->objects, ce_mob_object_new(vt));
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_object_lever_stats(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(mob_file);
        ce_mem_file_read(mem_file, mob_lever->stats, sizeof(uint32_t), 3);
    }

    void ce_mob_file_block_object_lever_state(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(mob_file);
        mob_lever->state = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_lever_state_count(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(mob_file);
        mob_lever->state_count = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_lever_cyclic(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(mob_file);
        mob_lever->cyclic = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_lever_door(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(mob_file);
        mob_lever->door = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_lever_recalc_graph(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_LEVER_CAST(mob_file);
        mob_lever->recalc_graph = ce_mem_file_read_u8(mem_file);
    }

    // trap

    void ce_mob_file_block_object_trap(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        ce_mob_object_vtable vt = { sizeof(ce_mob_trap), ce_mob_trap_dtor };
        ce_vector_push_back(mob_file->objects, ce_mob_object_new(vt));
        ce_mob_file_block_loop(mob_file, mem_file, size);
    }

    void ce_mob_file_block_object_trap_diplomacy(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_TRAP_CAST(mob_file);
        mob_trap->diplomacy = ce_mem_file_read_u32le(mem_file);
    }

    void ce_mob_file_block_object_trap_spell(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_TRAP_CAST(mob_file);
        mob_trap->spell = ce_mob_read_string(mem_file, size);
    }

    void ce_mob_file_block_object_trap_cast_interval(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_TRAP_CAST(mob_file);
        mob_trap->cast_interval = ce_mem_file_read_u32le(mem_file);
    }

    void ce_mob_file_block_object_trap_cast_once(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t)
    {
        CE_MOB_TRAP_CAST(mob_file);
        mob_trap->cast_once = ce_mem_file_read_u8(mem_file);
    }

    void ce_mob_file_block_object_trap_area(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_TRAP_CAST(mob_file);
        uint32_t count = ce_mem_file_read_u32le(mem_file);
        size -= 4;
        mob_trap->area = (ce_mob_trap_area*)ce_alloc(sizeof(ce_mob_trap_area) + size);
        mob_trap->area->size = size;
        mob_trap->area->count = count;
        ce_mem_file_read(mem_file, mob_trap->area->values, 1, size);
    }

    void ce_mob_file_block_object_trap_target(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
    {
        CE_MOB_TRAP_CAST(mob_file);
        uint32_t count = ce_mem_file_read_u32le(mem_file);
        size -= 4;
        mob_trap->target = (ce_mob_trap_target*)ce_alloc(sizeof(ce_mob_trap_target) + size);
        mob_trap->target->size = size;
        mob_trap->target->count = count;
        ce_mem_file_read(mem_file, mob_trap->target->values, 1, size);
    }

    typedef void (*ce_mob_file_block_callback)(ce_mob_file*, ce_mem_file*, size_t);

    typedef struct {
        uint32_t type;
        ce_mob_file_block_callback callback;
    } ce_mob_file_block_pair;

    const ce_mob_file_block_pair ce_mob_file_block_pairs[] = {
        {0xa000, ce_mob_file_block_main},
        {0xd000, ce_mob_file_block_quest},
        {0xc000, ce_mob_file_block_zonal},
        {0xacceeccb, ce_mob_file_block_text},
        {0xb000, ce_mob_file_block_object},
        {0xb001, ce_mob_file_block_object_object},
        {0xb00d, ce_mob_file_block_object_object_parts},
        {0xb011, ce_mob_file_block_object_object_owner},
        {0xb002, ce_mob_file_block_object_object_id},
        {0xb003, ce_mob_file_block_object_object_type},
        {0xb004, ce_mob_file_block_object_object_name},
        {0xb006, ce_mob_file_block_object_object_model_name},
        {0xb00e, ce_mob_file_block_object_object_parent_name},
        {0xb007, ce_mob_file_block_object_object_primary_texture},
        {0xb008, ce_mob_file_block_object_object_secondary_texture},
        {0xb00f, ce_mob_file_block_object_object_comment},
        {0xb009, ce_mob_file_block_object_object_position},
        {0xb00a, ce_mob_file_block_object_object_rotation},
        {0xb013, ce_mob_file_block_object_object_quest},
        {0xb014, ce_mob_file_block_object_object_shadow},
        {0xb012, ce_mob_file_block_object_object_parent_id},
        {0xb016, ce_mob_file_block_object_object_quest_info},
        {0xb00c, ce_mob_file_block_object_object_complection},
        {0xbbbb0000, ce_mob_file_block_object_unit},
        {0xbbbb000a, ce_mob_file_block_object_unit_need_import},
        {0xbbbb0002, ce_mob_file_block_object_unit_name},
        {0xbbbb0009, ce_mob_file_block_object_unit_armors},
        {0xbbbb0008, ce_mob_file_block_object_unit_weapons},
        {0xbbbb0007, ce_mob_file_block_object_unit_spells},
        {0xbbbb0006, ce_mob_file_block_object_unit_quick_items},
        {0xbbbb0005, ce_mob_file_block_object_unit_quest_items},
        {0xbbbb0004, ce_mob_file_block_object_unit_stats},
        {0xbbbc0000, ce_mob_file_block_object_unit_logic},
        {0xbbbc000b, ce_mob_file_block_object_unit_logic_alarm_condition},
        {0xbbbc000c, ce_mob_file_block_object_unit_logic_help},
        {0xbbbc0002, ce_mob_file_block_object_unit_logic_cyclic},
        {0xbbbc000e, ce_mob_file_block_object_unit_logic_aggression_mode},
        {0xbbbc000d, ce_mob_file_block_object_unit_logic_always_active},
        {0xbbbc0003, ce_mob_file_block_object_unit_logic_model},
        {0xbbbc0004, ce_mob_file_block_object_unit_logic_guard_radius},
        {0xbbbc000a, ce_mob_file_block_object_unit_logic_wait},
        {0xbbbc0005, ce_mob_file_block_object_unit_logic_guard_position},
        {0xbbbc0007, ce_mob_file_block_object_unit_logic_use},
        {0xbbbc0006, ce_mob_file_block_object_unit_logic_nalarm},
        {0xbbac0000, ce_mob_file_block_object_lever},
        {0xbbac0006, ce_mob_file_block_object_lever_stats},
        {0xbbac0002, ce_mob_file_block_object_lever_state},
        {0xbbac0003, ce_mob_file_block_object_lever_state_count},
        {0xbbac0004, ce_mob_file_block_object_lever_cyclic},
        {0xbbac0007, ce_mob_file_block_object_lever_door},
        {0xbbac0008, ce_mob_file_block_object_lever_recalc_graph},
        {0xbbab0000, ce_mob_file_block_object_trap},
        {0xbbab0001, ce_mob_file_block_object_trap_diplomacy},
        {0xbbab0002, ce_mob_file_block_object_trap_spell},
        {0xbbab0005, ce_mob_file_block_object_trap_cast_interval},
        {0xbbac0005, ce_mob_file_block_object_trap_cast_once},
        {0xbbab0003, ce_mob_file_block_object_trap_area},
        {0xbbab0004, ce_mob_file_block_object_trap_target},
        //{0xbbbf, ce_mob_file_block_object_flame},
        //{0xaa01, ce_mob_file_block_object_particle1},
        //{0xcc01, ce_mob_file_block_object_particle2},
        //{0xdd01, ce_mob_file_block_object_particle3},
    };

    ce_mob_file_block_callback ce_mob_choose_callback(uint32_t type)
    {
        for (size_t i = 0; i < sizeof(ce_mob_file_block_pairs) / sizeof(ce_mob_file_block_pairs[0]); ++i) {
            if (ce_mob_file_block_pairs[i].type == type) {
                return ce_mob_file_block_pairs[i].callback;
            }
        }
        return ce_mob_file_block_unknown;
    }

    void ce_mob_file_block_loop(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
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

            (*ce_mob_choose_callback(child_type))(mob_file, mem_file, child_size);
        }
    }

    ce_mob_file* ce_mob_file_open_mem_file(const char* name, ce_mem_file* mem_file)
    {
        ce_mob_file* mob_file = (ce_mob_file*)ce_alloc_zero(sizeof(ce_mob_file));
        mob_file->name = ce_string_new_str_n(name, std::min(strlen(name), strlen(name) - 4));

        ce_mob_file_block_loop(mob_file, mem_file, ce_mem_file_size(mem_file));

        return mob_file;
    }

    ce_mob_file* ce_mob_file_open(const char* path)
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

        ce_mob_file* mob_file = ce_mob_file_open_mem_file(name, mem_file);
        ce_mem_file_del(mem_file);

        return mob_file;
    }

    void ce_mob_file_close(ce_mob_file* mob_file)
    {
        if (NULL != mob_file) {
            if (NULL != mob_file->objects) {
                ce_vector_for_each(mob_file->objects, (void(*)(void*))ce_mob_object_del);
            }
            ce_vector_del(mob_file->objects);
            ce_string_del(mob_file->script);
            ce_string_del(mob_file->name);
            ce_free(mob_file, sizeof(ce_mob_file));
        }
    }
}
