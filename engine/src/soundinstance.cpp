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

#include "soundinstance.hpp"
#include "soundmixer.hpp"
#include "threadlock.hpp"

namespace cursedearth
{
    sound_instance_t::sound_instance_t(ce_sound_resource* resource):
        m_state(SOUND_INSTANCE_STATE_STOPPED),
        m_bytes_per_second_inv(1.0f / resource->sound_format.bytes_per_second),
        m_resource(resource),
        m_buffer(sound_mixer_t::instance()->make_buffer(resource->sound_format)),
        m_pause_condition(make_condition_variable()),
        m_thread("sound instance", [this]{execute();})
    {
    }

    sound_instance_t::~sound_instance_t()
    {
        m_thread.temp();
        ce_sound_resource_del(m_resource);
    }

    void sound_instance_t::change_state(sound_instance_state_t state)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ignore = lock;
        m_state = state;
        m_pause_condition->notify_all();
    }

    void sound_instance_t::pause()
    {
        m_buffer->sleep();
        thread_lock_t lock(m_mutex, m_pause_condition);
        m_pause_condition->wait(lock);
        m_buffer->wakeup();
    }

    void sound_instance_t::execute()
    {
        pause();
        while (true) {
            switch (m_state) {
            case SOUND_INSTANCE_STATE_PLAYING:
                if (!ce_sound_resource_read(m_resource, m_buffer)) {
                    change_state(SOUND_INSTANCE_STATE_STOPPED);
                }
                break;

            case SOUND_INSTANCE_STATE_PAUSED:
                pause();
                break;

            case SOUND_INSTANCE_STATE_STOPPED:
                ce_sound_resource_reset(m_resource);
                m_buffer->reset_granule_position();
                pause();
                break;
            }
        }
    }
}
