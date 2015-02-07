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

#include <vector>

#include "singleton.hpp"
#include "resfile.hpp"
#include "soundinstance.hpp"

namespace cursedearth
{
    class sound_manager_t: public singleton_t<sound_manager_t>
    {
    public:
        sound_manager_t();
        ~sound_manager_t();

        void advance(float elapsed);

        sound_object_t create_object(const char* name);
        sound_instance_ptr_t find_instance(sound_object_t);

    private:
        sound_object_t m_last_object;
        std::vector<ce_res_file*> m_res_files;
        std::vector<sound_instance_ptr_t> m_sound_instances;
    };
}

#endif
