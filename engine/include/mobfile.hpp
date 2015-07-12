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

#ifndef CE_MOBFILE_HPP
#define CE_MOBFILE_HPP

#include <cstdint>

#include <boost/filesystem/path.hpp>

#include <vector>
#include <string.hpp>
#include "memfile.hpp"

namespace cursedearth
{

    class mob_object {
        friend class mob_file;

    public:
        mob_object() {}
        virtual ~mob_object();

    public: // TODO protected?
        uint8_t owner, quest, shadow;
        uint32_t type, id, parent_id;
        float position[3];
        float rotation[4];
        float complection[3];
        std::vector<ce_string*>* parts = nullptr;
        ce_string* name;
        ce_string* model_name;
        ce_string* parent_name;
        ce_string* primary_texture;
        ce_string* secondary_texture;
        ce_string* comment;
        ce_string* quest_info;

    };

    class mob_unit_logic : public mob_object {
        friend class mob_file;

    public:
        mob_unit_logic() {}

    private:
        uint8_t alarm_condition;
        uint8_t aggression_mode;
        uint8_t always_active;
        uint8_t cyclic, use, nalarm;
        uint32_t model;
        float guard_radius;
        float guard_position[3];
        float wait, help;

    };

    class mob_unit : public mob_object {
        friend class mob_file;

    public:
        mob_unit() {}
        virtual ~mob_unit();

    private:
        uint8_t need_import;
        ce_string* name;
        std::vector<ce_string*>* armors = nullptr;
        std::vector<ce_string*>* weapons = nullptr;
        std::vector<ce_string*>* spells = nullptr;
        std::vector<ce_string*>* quick_items = nullptr;
        std::vector<ce_string*>* quest_items = nullptr;
        std::vector<mob_unit_logic*>* logics = nullptr;

    };

    class mob_lever : public mob_object {
        friend class mob_file;

    public:
        mob_lever() {}

    private:
        uint32_t stats[3];
        uint8_t state, state_count;
        uint8_t cyclic, door;
        uint8_t recalc_graph;
    };

    class mob_trap_area : public mob_object {
        friend class mob_file;

    public:
        mob_trap_area() {}

    private:
        size_t size;
        uint32_t count;
        float* values[3]; // x, z, area
    };

    class mob_trap_target : public mob_object {
        friend class mob_file;

    public:
        mob_trap_target() {}

    private:
        size_t size;
        uint32_t count;
        float* values[2]; // x, z
    };

    class mob_trap : public mob_object {
        friend class mob_file;

    public:
        mob_trap() {}
        virtual ~mob_trap();

    private:
        uint8_t cast_once;
        uint32_t diplomacy, cast_interval;
        ce_string* spell;
        mob_trap_area* area;
        mob_trap_target* target;
    };

    /**
     * @brief doc/formats/mob.txt
     */
    class mob_file {

    public:
        mob_file(const boost::filesystem::path&);
        ~mob_file();

        ce_string* get_name();
        ce_string* get_script();
        std::vector<mob_object*>* get_objects();

    private:
        typedef void (mob_file::*block_callback)(ce_mem_file*, size_t);

        ce_string* name;
        ce_string* script;
        std::vector<mob_object*>* objects;

        void decrypt_script(char* data, size_t size, uint32_t key);
        ce_string* read_string(ce_mem_file* mem_file, size_t size);

        void block_loop(ce_mem_file*, size_t);
        void block_unknown(ce_mem_file* mem_file, size_t size);
        void block_main(ce_mem_file* mem_file, size_t size);
        void block_quest(ce_mem_file* mem_file, size_t size);
        void block_zonal(ce_mem_file* mem_file, size_t size);
        void block_text(ce_mem_file* mem_file, size_t size);
        void block_object(ce_mem_file* mem_file, size_t size);

        // object

        void block_object_object(ce_mem_file* mem_file, size_t size);
        void block_object_object_parts(ce_mem_file* mem_file, size_t size);
        void block_object_object_owner(ce_mem_file* mem_file, size_t);
        void block_object_object_id(ce_mem_file* mem_file, size_t);
        void block_object_object_type(ce_mem_file* mem_file, size_t);
        void block_object_object_name(ce_mem_file* mem_file, size_t size);
        void block_object_object_model_name(ce_mem_file* mem_file, size_t size);
        void block_object_object_parent_name(ce_mem_file* mem_file, size_t size);
        void block_object_object_primary_texture(ce_mem_file* mem_file, size_t size);
        void block_object_object_secondary_texture(ce_mem_file* mem_file, size_t size);
        void block_object_object_comment(ce_mem_file* mem_file, size_t size);
        void block_object_object_position(ce_mem_file* mem_file, size_t);
        void block_object_object_rotation(ce_mem_file* mem_file, size_t);
        void block_object_object_quest(ce_mem_file* mem_file, size_t);
        void block_object_object_shadow(ce_mem_file* mem_file, size_t);
        void block_object_object_parent_id(ce_mem_file* mem_file, size_t);
        void block_object_object_quest_info(ce_mem_file* mem_file, size_t size);
        void block_object_object_complection(ce_mem_file* mem_file, size_t);

        // unit

        void block_object_unit(ce_mem_file* mem_file, size_t size);
        void block_object_unit_need_import(ce_mem_file* mem_file, size_t);
        void block_object_unit_name(ce_mem_file* mem_file, size_t size);
        void block_object_unit_armors(ce_mem_file* mem_file, size_t size);
        void block_object_unit_weapons(ce_mem_file* mem_file, size_t size);
        void block_object_unit_spells(ce_mem_file* mem_file, size_t size);
        void block_object_unit_quick_items(ce_mem_file* mem_file, size_t size);
        void block_object_unit_quest_items(ce_mem_file* mem_file, size_t size);
        void block_object_unit_stats(ce_mem_file* mem_file, size_t size);

        // unit logic

        void block_object_unit_logic(ce_mem_file* mem_file, size_t size);
        void block_object_unit_logic_alarm_condition(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_help(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_cyclic(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_aggression_mode(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_always_active(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_model(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_guard_radius(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_wait(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_guard_position(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_use(ce_mem_file* mem_file, size_t);
        void block_object_unit_logic_nalarm(ce_mem_file* mem_file, size_t);

        // lever

        void block_object_lever(ce_mem_file* mem_file, size_t size);
        void block_object_lever_stats(ce_mem_file* mem_file, size_t);
        void block_object_lever_state(ce_mem_file* mem_file, size_t);
        void block_object_lever_state_count(ce_mem_file* mem_file, size_t);
        void block_object_lever_cyclic(ce_mem_file* mem_file, size_t);
        void block_object_lever_door(ce_mem_file* mem_file, size_t);
        void block_object_lever_recalc_graph(ce_mem_file* mem_file, size_t);

        // trap

        void block_object_trap(ce_mem_file* mem_file, size_t size);
        void block_object_trap_diplomacy(ce_mem_file* mem_file, size_t);
        void block_object_trap_spell(ce_mem_file* mem_file, size_t size);
        void block_object_trap_cast_interval(ce_mem_file* mem_file, size_t);
        void block_object_trap_cast_once(ce_mem_file* mem_file, size_t);
        void block_object_trap_area(ce_mem_file* mem_file, size_t size);
        void block_object_trap_target(ce_mem_file* mem_file, size_t size);

        block_callback choose_callback(uint32_t type);

    private:
        typedef struct {
            uint32_t type;
            block_callback callback;
        } block_pair;

        const block_pair block_pairs[58] = {
            {0xa000, &mob_file::block_main},
            {0xd000, &mob_file::block_quest},
            {0xc000, &mob_file::block_zonal},
            {0xacceeccb, &mob_file::block_text},
            {0xb000, &mob_file::block_object},
            {0xb001, &mob_file::block_object_object},
            {0xb00d, &mob_file::block_object_object_parts},
            {0xb011, &mob_file::block_object_object_owner},
            {0xb002, &mob_file::block_object_object_id},
            {0xb003, &mob_file::block_object_object_type},
            {0xb004, &mob_file::block_object_object_name},
            {0xb006, &mob_file::block_object_object_model_name},
            {0xb00e, &mob_file::block_object_object_parent_name},
            {0xb007, &mob_file::block_object_object_primary_texture},
            {0xb008, &mob_file::block_object_object_secondary_texture},
            {0xb00f, &mob_file::block_object_object_comment},
            {0xb009, &mob_file::block_object_object_position},
            {0xb00a, &mob_file::block_object_object_rotation},
            {0xb013, &mob_file::block_object_object_quest},
            {0xb014, &mob_file::block_object_object_shadow},
            {0xb012, &mob_file::block_object_object_parent_id},
            {0xb016, &mob_file::block_object_object_quest_info},
            {0xb00c, &mob_file::block_object_object_complection},
            {0xbbbb0000, &mob_file::block_object_unit},
            {0xbbbb000a, &mob_file::block_object_unit_need_import},
            {0xbbbb0002, &mob_file::block_object_unit_name},
            {0xbbbb0009, &mob_file::block_object_unit_armors},
            {0xbbbb0008, &mob_file::block_object_unit_weapons},
            {0xbbbb0007, &mob_file::block_object_unit_spells},
            {0xbbbb0006, &mob_file::block_object_unit_quick_items},
            {0xbbbb0005, &mob_file::block_object_unit_quest_items},
            {0xbbbb0004, &mob_file::block_object_unit_stats},
            {0xbbbc0000, &mob_file::block_object_unit_logic},
            {0xbbbc000b, &mob_file::block_object_unit_logic_alarm_condition},
            {0xbbbc000c, &mob_file::block_object_unit_logic_help},
            {0xbbbc0002, &mob_file::block_object_unit_logic_cyclic},
            {0xbbbc000e, &mob_file::block_object_unit_logic_aggression_mode},
            {0xbbbc000d, &mob_file::block_object_unit_logic_always_active},
            {0xbbbc0003, &mob_file::block_object_unit_logic_model},
            {0xbbbc0004, &mob_file::block_object_unit_logic_guard_radius},
            {0xbbbc000a, &mob_file::block_object_unit_logic_wait},
            {0xbbbc0005, &mob_file::block_object_unit_logic_guard_position},
            {0xbbbc0007, &mob_file::block_object_unit_logic_use},
            {0xbbbc0006, &mob_file::block_object_unit_logic_nalarm},
            {0xbbac0000, &mob_file::block_object_lever},
            {0xbbac0006, &mob_file::block_object_lever_stats},
            {0xbbac0002, &mob_file::block_object_lever_state},
            {0xbbac0003, &mob_file::block_object_lever_state_count},
            {0xbbac0004, &mob_file::block_object_lever_cyclic},
            {0xbbac0007, &mob_file::block_object_lever_door},
            {0xbbac0008, &mob_file::block_object_lever_recalc_graph},
            {0xbbab0000, &mob_file::block_object_trap},
            {0xbbab0001, &mob_file::block_object_trap_diplomacy},
            {0xbbab0002, &mob_file::block_object_trap_spell},
            {0xbbab0005, &mob_file::block_object_trap_cast_interval},
            {0xbbac0005, &mob_file::block_object_trap_cast_once},
            {0xbbab0003, &mob_file::block_object_trap_area},
            {0xbbab0004, &mob_file::block_object_trap_target},
            //{0xbbbf, &mob_file::block_object_flame},
            //{0xaa01, &mob_file::block_object_particle1},
            //{0xcc01, &mob_file::block_object_particle2},
            //{0xdd01, &mob_file::block_object_particle3},
        };

    };

}

#endif
