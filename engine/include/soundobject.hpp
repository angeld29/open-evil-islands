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

#ifndef CE_SOUNDOBJECT_HPP
#define CE_SOUNDOBJECT_HPP

#include <string>

namespace cursedearth
{
    typedef unsigned long sound_object_t;

    sound_object_t make_sound_object(const std::string&);
    void remove_sound_object(sound_object_t);

    bool sound_object_is_valid(sound_object_t);

    void sound_object_advance(sound_object_t, float elapsed);

    bool sound_object_is_stopped(sound_object_t);
    void play_sound_object(sound_object_t);
    void pause_sound_object(sound_object_t);
    void stop_sound_object(sound_object_t);

    int get_sound_object_state(sound_object_t);
    float get_sound_object_time(sound_object_t);
}

#endif
