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

#include "soundmanager.hpp"
#include "soundobject.hpp"

namespace cursedearth
{
    sound_object_t make_sound_object(const char* name)
    {
        return sound_manager_t::instance()->create_object(name);
    }

    void remove_sound_object(sound_object_t)
    {
    }

    bool sound_object_is_valid(sound_object_t sound_object)
    {
        return 0 != sound_object && sound_manager_t::instance()->find_instance(sound_object);
    }

    void sound_object_advance(sound_object_t sound_object, float elapsed)
    {
        if (sound_instance_ptr_t sound_instance = sound_manager_t::instance()->find_instance(sound_object)) {
            sound_instance->advance(elapsed);
        }
    }

    bool sound_object_is_stopped(sound_object_t sound_object)
    {
        return SOUND_INSTANCE_STATE_STOPPED == get_sound_object_state(sound_object);
    }

    void play_sound_object(sound_object_t sound_object)
    {
        if (sound_instance_ptr_t sound_instance = sound_manager_t::instance()->find_instance(sound_object)) {
            sound_instance->change_state(SOUND_INSTANCE_STATE_PLAYING);
        }
    }

    void pause_sound_object(sound_object_t sound_object)
    {
        if (sound_instance_ptr_t sound_instance = sound_manager_t::instance()->find_instance(sound_object)) {
            sound_instance->change_state(SOUND_INSTANCE_STATE_PAUSED);
        }
    }

    void stop_sound_object(sound_object_t sound_object)
    {
        if (sound_instance_ptr_t sound_instance = sound_manager_t::instance()->find_instance(sound_object)) {
            sound_instance->change_state(SOUND_INSTANCE_STATE_STOPPED);
        }
    }

    int get_sound_object_state(sound_object_t sound_object)
    {
        sound_instance_ptr_t sound_instance = sound_manager_t::instance()->find_instance(sound_object);
        return sound_instance ? sound_instance->state() : SOUND_INSTANCE_STATE_STOPPED;
    }

    float get_sound_object_time(sound_object_t sound_object)
    {
        sound_instance_ptr_t sound_instance = sound_manager_t::instance()->find_instance(sound_object);
        return sound_instance ? sound_instance->time() : 0.0f;
    }
}
