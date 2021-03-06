/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_SOUNDMIXER_HPP
#define CE_SOUNDMIXER_HPP

#include "makeunique.hpp"
#include "singleton.hpp"
#include "soundbuffer.hpp"

#include <list>

namespace cursedearth
{
    class sound_mixer_t final: public singleton_t<sound_mixer_t>
    {
    public:
        sound_mixer_t();
        ~sound_mixer_t();

        sound_buffer_ptr_t make_buffer(const sound_format_t&);

    private:
        void execute();

    private:
        std::list<sound_buffer_ptr_t> m_buffers;
        std::mutex m_mutex;
        thread_t m_thread;
    };

    typedef std::unique_ptr<sound_mixer_t> sound_mixer_ptr_t;

    inline sound_mixer_ptr_t make_sound_mixer()
    {
        return make_unique<sound_mixer_t>();
    }
}

#endif
