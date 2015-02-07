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
#include "soundbuffer.hpp"

namespace cursedearth
{
    sound_buffer_t::sound_buffer_t(const sound_format_t& format):
        m_format(format),
        m_buffer(format.sample_size * SOUND_CAPABILITY_SAMPLES_IN_BLOCK * SOUND_CAPABILITY_BLOCK_COUNT)
    {
    }

    bool sound_buffer_t::push(const void* array, bool wait)
    {
        const size_t available = m_buffer.write_available();
        if (available < m_format.sample_size && !wait) {
            return false;
        }
        m_buffer.write(array, m_format.sample_size);
        return true;
    }

    size_t sound_buffer_t::push(const void* array, size_t n, bool wait)
    {
        const size_t available = m_buffer.write_available();
        size_t size = m_format.sample_size * n;
        if (available < size && !wait) {
            size = available - (available % m_format.sample_size);
        }
        m_buffer.write(array, size);
        return size;
    }

    bool sound_buffer_t::pop(void* array, bool wait)
    {
        const size_t available = m_buffer.read_available();
        if (available < m_format.sample_size && !wait) {
            return false;
        }
        m_buffer.read(array, m_format.sample_size);
        return true;
    }

    size_t sound_buffer_t::pop(void* array, size_t n, bool wait)
    {
        const size_t available = m_buffer.read_available();
        size_t size = m_format.sample_size * n;
        if (available < size && !wait) {
            size = available - (available % m_format.sample_size);
        }
        m_buffer.read(array, size);
        return size;
    }
}
