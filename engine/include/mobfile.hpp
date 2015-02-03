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

/**
 * doc/formats/mob.txt
 */

#ifndef CE_MOBFILE_HPP
#define CE_MOBFILE_HPP

#include <cstdint>

#include "vector.hpp"
#include "string.hpp"

namespace cursedearth
{
    typedef struct ce_mob_object ce_mob_object;

    typedef struct {
        size_t size;
        void (*dtor)(ce_mob_object* mob_object);
    } ce_mob_object_vtable;

    struct ce_mob_object {
        uint8_t owner, quest, shadow;
        uint32_t type, id, parent_id;
        float position[3];
        float rotation[4];
        float complection[3];
        ce_vector* parts;
        ce_string* name;
        ce_string* model_name;
        ce_string* parent_name;
        ce_string* primary_texture;
        ce_string* secondary_texture;
        ce_string* comment;
        ce_string* quest_info;
        ce_mob_object_vtable vtable;
        void* impl;
    };

    typedef struct {
        uint8_t alarm_condition;
        uint8_t aggression_mode;
        uint8_t always_active;
        uint8_t cyclic, use, nalarm;
        uint32_t model;
        float guard_radius;
        float guard_position[3];
        float wait, help;
    } ce_mob_unit_logic;

    typedef struct {
        uint8_t need_import;
        ce_string* name;
        ce_vector* armors;
        ce_vector* weapons;
        ce_vector* spells;
        ce_vector* quick_items;
        ce_vector* quest_items;
        ce_vector* logics;
    } ce_mob_unit;

    typedef struct {
        uint32_t stats[3];
        uint8_t state, state_count;
        uint8_t cyclic, door;
        uint8_t recalc_graph;
    } ce_mob_lever;

    typedef struct {
        size_t size;
        uint32_t count;
        float* values[3]; // x, z, area
    } ce_mob_trap_area;

    typedef struct {
        size_t size;
        uint32_t count;
        float* values[2]; // x, z
    } ce_mob_trap_target;

    typedef struct {
        uint8_t cast_once;
        uint32_t diplomacy, cast_interval;
        ce_string* spell;
        ce_mob_trap_area* area;
        ce_mob_trap_target* target;
    } ce_mob_trap;

    typedef struct {
        ce_string* name;
        ce_string* script;
        ce_vector* objects;
    } ce_mob_file;

    ce_mob_file* ce_mob_file_open(const char* path);
    void ce_mob_file_close(ce_mob_file* mob_file);
}

#endif
