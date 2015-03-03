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

#ifndef CE_SOUNDDEVICE_HPP
#define CE_SOUNDDEVICE_HPP

#include "makeunique.hpp"
#include "soundblock.hpp"

namespace cursedearth
{
    class sound_device_t: untransferable_t
    {
    public:
        explicit sound_device_t(const sound_format_t& format): m_format(format) {}
        virtual ~sound_device_t() = default;

        virtual void write(const sound_block_ptr_t&) = 0;

    protected:
        const sound_format_t m_format;
    };

    class null_sound_device_t final: public sound_device_t
    {
    public:
        explicit null_sound_device_t(const sound_format_t&);

        virtual void write(const sound_block_ptr_t&) final;
    };

    typedef std::shared_ptr<sound_device_t> sound_device_ptr_t;

    sound_device_ptr_t make_sound_device(const sound_format_t&);

    inline sound_device_ptr_t make_null_sound_device(const sound_format_t& format)
    {
        return std::make_shared<null_sound_device_t>(format);
    }
}

#endif
