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

#include "soundblock.hpp"
#include "makeunique.hpp"

#include <cassert>
#include <algorithm>

namespace cursedearth
{
    sound_block_t::sound_block_t(const sound_format_t& format):
        m_format(format),
        m_capacity(m_format.sample_size * sound_options_t::samples_in_block),
        m_data(make_unique<uint8_t[]>(m_capacity))
    {
    }

    size_t sound_block_t::write(const uint8_t* data, size_t size)
    {
        assert(0 == size % m_format.sample_size);
        assert(m_write_position <= m_capacity);
        size = std::min(size, write_size());
        std::copy_n(data, size, m_data.get() + m_write_position);
        m_write_position += size;
        return size;
    }

    size_t sound_block_t::read(uint8_t* data, size_t size)
    {
        assert(0 == size % m_format.sample_size);
        assert(m_read_position <= m_write_position);
        size = std::min(size, read_size());
        std::copy_n(m_data.get() + m_read_position, size, data);
        m_read_position += size;
        return size;
    }

    std::pair<const uint8_t*, size_t> sound_block_t::read_raw()
    {
        assert(m_read_position <= m_write_position);
        auto pair = std::make_pair(m_data.get() + m_read_position, read_size());
        m_read_position = m_write_position;
        return std::move(pair);
    }

    void sound_block_t::reset()
    {
        m_write_position = 0;
        m_read_position = 0;
    }
}
