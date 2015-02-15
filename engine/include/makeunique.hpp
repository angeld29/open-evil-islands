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

#ifndef CE_MAKEUNIQUE_HPP
#define CE_MAKEUNIQUE_HPP

#include "standardheaders.hpp"
#include "thirdpartyheaders.hpp"

#if __cplusplus >= 201402L
#error std::make_unique is available now
#endif

namespace cursedearth
{
    template<class T>
    struct _Unique_if
    {
        typedef std::unique_ptr<T> _Single_object;
    };

    template<class T>
    struct _Unique_if<T[]>
    {
        typedef std::unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N>
    struct _Unique_if<T[N]>
    {
        typedef void _Known_bound;
    };

    template<class T, class... Args>
    typename _Unique_if<T>::_Single_object make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template<class T>
    typename _Unique_if<T>::_Unknown_bound make_unique(size_t n)
    {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[n]());
    }

    template<class T, class... Args>
    typename _Unique_if<T>::_Known_bound make_unique(Args&&...) = delete;
}

#endif
