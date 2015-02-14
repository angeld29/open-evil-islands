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

#ifndef CE_UNTRANSFERABLE_HPP
#define CE_UNTRANSFERABLE_HPP

#include "standardheaders.hpp"

namespace cursedearth
{
    class uncopyable_t
    {
    protected:
        uncopyable_t() = default;
        ~uncopyable_t() = default;

        uncopyable_t(const uncopyable_t&) = delete;
        uncopyable_t& operator=(const uncopyable_t&) = delete;
    };

    class unmovable_t
    {
    protected:
        unmovable_t() = default;
        ~unmovable_t() = default;

        unmovable_t(unmovable_t&&) = delete;
        unmovable_t& operator=(unmovable_t&&) = delete;
    };

    class untransferable_t: uncopyable_t, unmovable_t
    {
    protected:
        untransferable_t() = default;
        ~untransferable_t() = default;
    };
}

#endif
