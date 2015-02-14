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

#ifndef CE_SOUNDBLOCK_HPP
#define CE_SOUNDBLOCK_HPP

#include "soundformat.hpp"

namespace cursedearth
{
    class sound_block_t final: untransferable_t
    {
    public:
        explicit sound_block_t(const sound_format_t&);

        const sound_format_t& format() const { return m_format; }

        size_t write(const uint8_t*, size_t);
        size_t read(uint8_t*, size_t);

        std::pair<const uint8_t*, size_t> read_raw();

        void reset();

    private:
        const sound_format_t m_format;
        const size_t m_capacity;
        size_t m_write_position = 0, m_read_position = 0;
        std::unique_ptr<uint8_t[]> m_data;
    };

    typedef std::shared_ptr<sound_block_t> sound_block_ptr_t;
}

#endif
