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

#ifndef CE_SOUNDBUFFER_HPP
#define CE_SOUNDBUFFER_HPP

#include <memory>

#include <boost/noncopyable.hpp>

#include "ringbuffer.hpp"
#include "soundcapabilities.hpp"
#include "soundformat.hpp"

namespace cursedearth
{
    class sound_buffer_t final: boost::noncopyable
    {
    public:
        explicit sound_buffer_t(const sound_format_t&);

        const sound_format_t& format() const { return m_format; }

        // pushes sample(s) to the buffer
        bool push(const void*, bool wait = true);
        size_t push(const void*, size_t n, bool wait = true);

        // pops sample(s) from buffer
        bool pop(void*, bool wait = true);
        size_t pop(void*, size_t n, bool wait = true);

    private:
        const sound_format_t m_format;
        ring_buffer_t m_buffer;
    };

    typedef std::shared_ptr<sound_buffer_t> sound_buffer_ptr_t;
}

#endif
