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

#ifndef CE_SOUNDMANAGER_HPP
#define CE_SOUNDMANAGER_HPP

#include "vector.hpp"
#include "soundinstance.hpp"

namespace cursedearth
{
    extern struct ce_sound_manager {
        ce_sound_object last_sound_object;
        ce_vector* res_files;
        ce_vector* sound_instances;
    }* ce_sound_manager;

    void ce_sound_manager_init(void);
    void ce_sound_manager_term(void);

    void ce_sound_manager_advance(float elapsed);

    ce_sound_object ce_sound_manager_create_object(const char* name);
    ce_sound_instance* ce_sound_manager_find_instance(ce_sound_object sound_object);
}

#endif
