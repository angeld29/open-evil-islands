/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_SINGLETON_HPP
#define CE_SINGLETON_HPP

#include "untransferable.hpp"

#include <cassert>
#include <atomic>

namespace cursedearth
{
    template <typename T>
    class singleton_t: untransferable_t
    {
    protected:
        explicit singleton_t(T* instance);
        ~singleton_t();

    public:
        static T* instance();

    private:
        static std::atomic<T*> m_instance;
    };

    template <typename T>
    singleton_t<T>::singleton_t(T* instance)
    {
        const T* old_instance = m_instance.exchange(instance);
        assert(nullptr == old_instance && "only one instance of this class is permitted");
    }

    template <typename T>
    singleton_t<T>::~singleton_t()
    {
        const T* old_instance = m_instance.exchange(nullptr);
        assert(nullptr != old_instance && "the singleton instance is invalid");
    }

    template <typename T>
    T* singleton_t<T>::instance()
    {
        assert(nullptr != m_instance.load() && "the singleton has not yet been created");
        return m_instance.load();
    }

    template <typename T>
    std::atomic<T*> singleton_t<T>::m_instance;
}

#endif
