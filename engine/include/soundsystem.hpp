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

#ifndef CE_SOUNDSYSTEM_HPP
#define CE_SOUNDSYSTEM_HPP

#include "soundbuffer.hpp"

namespace cursedearth
{
    class sound_system_t final: public singleton_t<sound_system_t>
    {
    public:
        sound_system_t();
        virtual ~sound_system_t();

        const sound_format_t& format() const { return m_format; }

        sound_block_ptr_t map();
        void unmap(const sound_block_ptr_t&);

    private:
        void execute();

        virtual void write(const sound_block_ptr_t&) = 0;

    private:
        const sound_format_t m_format;
        sound_buffer_ptr_t m_buffer;
        interruptible_thread_t m_thread;
    };

    typedef std::unique_ptr<sound_system_t> sound_system_ptr_t;

    sound_system_ptr_t make_sound_system();
}

#endif
