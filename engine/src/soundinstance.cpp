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

#include "logging.hpp"
#include "soundmixer.hpp"
#include "soundinstance.hpp"

namespace cursedearth
{
    sound_instance_t::sound_instance_t(ce_sound_resource* resource):
        m_resource(resource),
        m_buffer(sound_mixer_t::instance()->make_buffer(resource->sound_format)),
        m_state(SOUND_INSTANCE_STATE_STOPPED),
        m_time(0.0f),
        m_done(false),
        m_thread(ce_thread_new((void(*)())execute, this))
    {
    }

    sound_instance_t::~sound_instance_t()
    {
        m_done = true;
        ce_thread_wait(m_thread);
        ce_thread_del(m_thread);
        ce_sound_resource_del(m_resource);
    }

    void sound_instance_t::advance(float)
    {
    }

    void sound_instance_t::change_state(sound_instance_state_t state)
    {
        m_state = state;
    }

    void sound_instance_t::execute_playing(sound_instance_t* instance)
    {
        if (ce_sound_resource_read(instance->m_resource, instance->m_buffer)) {
            instance->m_time = instance->m_resource->time;
        } else {
            instance->change_state(SOUND_INSTANCE_STATE_STOPPED);
        }
    }

    void sound_instance_t::execute(sound_instance_t* instance)
    {
        while (!instance->m_done) {
            switch (instance->m_state) {
            case SOUND_INSTANCE_STATE_PLAYING:
                execute_playing(instance);
                break;
            case SOUND_INSTANCE_STATE_PAUSED:
                break;
            case SOUND_INSTANCE_STATE_STOPPED:
                instance->m_time = 0.0f;
                ce_sound_resource_reset(instance->m_resource);
                break;
            }
        }
    }
}
