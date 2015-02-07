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

#ifndef CE_SOUNDMIXER_HPP
#define CE_SOUNDMIXER_HPP

#include <list>

#include "singleton.hpp"
#include "thread.hpp"
#include "soundbuffer.hpp"

namespace cursedearth
{
    class sound_mixer_t: public singleton_t<sound_mixer_t>
    {
    public:
        sound_mixer_t();
        ~sound_mixer_t();

        sound_buffer_ptr_t make_buffer(const sound_format_t&);

    private:
        static void exec(sound_mixer_t*);

    private:
        std::list<sound_buffer_ptr_t> m_buffers;
        std::atomic<bool> m_done;
        ce_mutex* m_mutex;
        ce_thread* m_thread;
    };
}

#endif
