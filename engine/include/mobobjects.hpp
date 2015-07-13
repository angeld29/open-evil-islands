#ifndef MOBOBJECTS
#define MOBOBJECTS

#include <cstdint>
#include <vector>

#include "string.hpp"
#include "ScriptObject.hpp"

namespace cursedearth {

    class mob_object : public EIScript::ScriptObject {
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

        uint8_t get_state();

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
}

#endif // MOBOBJECTS

