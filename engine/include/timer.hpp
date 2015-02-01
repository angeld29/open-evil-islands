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

#ifndef CE_TIMER_HPP
#define CE_TIMER_HPP

#include <memory>

#include <boost/noncopyable.hpp>

namespace cursedearth
{
    class timer_t: boost::noncopyable
    {
    public:
        virtual ~timer_t() = 0;

        virtual void start() = 0;
        virtual float advance() = 0;
        virtual float elapsed() const = 0;
    };

    typedef std::shared_ptr<timer_t> timer_ptr_t;
    typedef std::shared_ptr<const timer_t> timer_const_ptr_t;

    timer_ptr_t make_timer();
}

#endif /* CE_TIMER_HPP */
