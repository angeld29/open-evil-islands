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

#include "ringbuffer.hpp"
#include "soundblock.hpp"

namespace cursedearth
{
    class sound_buffer_t final: boost::noncopyable
    {
    public:
        explicit sound_buffer_t(const sound_format_t&);

        const sound_format_t& format() const { return m_format; }

        void push(const sound_block_ptr_t&);
        sound_block_ptr_t pop();

        bool try_read_one_sample(uint8_t[sound_capabilities_t::max_sample_size]);

        sound_block_ptr_t acquire();
        void release(const sound_block_ptr_t&);

    private:
        const sound_format_t m_format;
        sound_block_ptr_t m_current_block;
        std::mutex m_mutex;
        std::vector<sound_block_ptr_t> m_blocks;
        ring_buffer_t<sound_block_ptr_t, sound_capabilities_t::block_count> m_buffer;
    };

    typedef std::shared_ptr<sound_buffer_t> sound_buffer_ptr_t;
}

#endif
