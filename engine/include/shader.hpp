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

#ifndef CE_SHADER_HPP
#define CE_SHADER_HPP

#include <memory>
#include <stdexcept>
#include <vector>

#include <boost/noncopyable.hpp>

namespace cursedearth
{
    typedef std::shared_ptr<class shader_t> shader_ptr_t;
    typedef std::shared_ptr<const class shader_t> shader_const_ptr_t;

    class shader_t: boost::noncopyable
    {
    public:
        enum type_t
        {
            TYPE_VERTEX,
            TYPE_FRAGMENT
        };

        struct info_t
        {
            type_t shader_type;
            size_t resource_index;
        };

        class not_available_error_t: public std::runtime_error
        {
        public:
            explicit not_available_error_t(const std::string& message): std::runtime_error(message) {}
        };

    public:
        virtual ~shader_t() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        virtual bool is_valid() = 0;

    protected:
        std::string m_name;
    };

    shader_ptr_t make_shader(const std::string& name, const std::vector<shader_t::info_t>&); // throw(not_available_error_t)
}

#endif /* CE_SHADER_HPP */
