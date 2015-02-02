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

#ifndef CE_TEXTURE_HPP
#define CE_TEXTURE_HPP

#include <memory>
#include <string>

#include <boost/noncopyable.hpp>

#include "mmp.hpp"

namespace cursedearth
{
    typedef std::shared_ptr<class texture_t> texture_ptr_t;
    typedef std::shared_ptr<const class texture_t> texture_const_ptr_t;

    class texture_t: boost::noncopyable
    {
    public:
        enum wrap_mode_t
        {
            WRAP_MODE_REPEAT,
            WRAP_MODE_CLAMP,
            WRAP_MODE_CLAMP_TO_EDGE,
            WRAP_MODE_COUNT
        };

    public:
        virtual ~texture_t() = default;

        virtual bool is_valid() = 0;
        virtual bool is_equal(const texture_const_ptr_t&) = 0;

        virtual void replace(ce_mmpfile*) = 0;
        virtual void wrap(wrap_mode_t) = 0;

        virtual void bind() = 0;
        virtual void unbind() = 0;

    protected:
        std::string m_name;
        unsigned int m_width, m_height;
    };

    texture_ptr_t make_texture(ce_mmpfile* mmpfile);
}

#endif /* CE_TEXTURE_HPP */
