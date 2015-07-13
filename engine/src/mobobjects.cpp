#include "mobobjects.hpp"

namespace cursedearth {

    void delete_ce_string_vector(std::vector<ce_string*>* vector){
        for(auto& str : *vector) {
           ce_string_del(str);
        }
        delete vector;
    }

    // base object

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

    // unit

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

    // lever

    uint8_t mob_lever::get_state()
    {
        return state;
    }

    // trap

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

}
