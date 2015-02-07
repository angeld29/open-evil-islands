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

#ifndef CE_SOUNDINSTANCE_HPP
#define CE_SOUNDINSTANCE_HPP

#include <atomic>
#include <memory>

#include <boost/noncopyable.hpp>

#include "thread.hpp"
#include "soundobject.hpp"
#include "soundresource.hpp"

namespace cursedearth
{
    enum sound_instance_state_t {
        SOUND_INSTANCE_STATE_STOPPED,
        SOUND_INSTANCE_STATE_PAUSED,
        SOUND_INSTANCE_STATE_PLAYING
    };

    class sound_instance_t: boost::noncopyable
    {
    public:
        sound_instance_t(sound_object_t, ce_sound_resource*);
        ~sound_instance_t();

        sound_object_t sound_object() const { return m_object; }
        sound_instance_state_t state() const { return m_state; }
        float time() const { return m_time; }

        void advance(float elapsed);
        void change_state(sound_instance_state_t);

    private:
        static void exec_playing(sound_instance_t*);
        static void exec(sound_instance_t*);

    private:
        const sound_object_t m_object;
        ce_sound_resource* m_resource;
        sound_buffer_ptr_t m_buffer;
        std::atomic<sound_instance_state_t> m_state;
        std::atomic<float> m_time;
        std::atomic<bool> m_done;
        ce_thread* m_thread;
    };

    typedef std::shared_ptr<sound_instance_t> sound_instance_ptr_t;
}

#endif
