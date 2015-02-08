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

#ifndef CE_BYTEORDER_HPP
#define CE_BYTEORDER_HPP

#include <cstdint>
#include <type_traits>

namespace cursedearth
{
    enum class endian_t {
        LITTLE,
        BIG,
        MIDDLE // this ordering is known as PDP
    };

    endian_t host_order();

    /**
     * @brief convert values between host (cpu) and big-/little-endian byte order
     */

    void do_cpu2le(uint16_t&);
    void do_cpu2le(uint32_t&);
    void do_cpu2le(uint64_t&);

    inline void do_cpu2le(uint8_t&) {}

    template <typename T>
    inline typename std::enable_if<std::is_unsigned<T>::value, T>::type cpu2le(T value)
    {
        do_cpu2le(value);
        return value;
    }

    template <typename T, typename U = typename std::make_unsigned<T>::type>
    inline typename std::enable_if<std::is_signed<T>::value, T>::type cpu2le(T value)
    {
        do_cpu2le(*reinterpret_cast<U*>(&value));
        return value;
    }

    void do_le2cpu(uint16_t&);
    void do_le2cpu(uint32_t&);
    void do_le2cpu(uint64_t&);

    inline void do_le2cpu(uint8_t&) {}

    template <typename T>
    inline typename std::enable_if<std::is_unsigned<T>::value, T>::type le2cpu(T value)
    {
        do_le2cpu(value);
        return value;
    }

    template <typename T, typename U = typename std::make_unsigned<T>::type>
    inline typename std::enable_if<std::is_signed<T>::value, T>::type le2cpu(T value)
    {
        do_le2cpu(*reinterpret_cast<U*>(&value));
        return value;
    }

    void do_cpu2be(uint16_t&);
    void do_cpu2be(uint32_t&);
    void do_cpu2be(uint64_t&);

    inline void do_cpu2be(uint8_t&) {}

    template <typename T>
    inline typename std::enable_if<std::is_unsigned<T>::value, T>::type cpu2be(T value)
    {
        do_cpu2be(value);
        return value;
    }

    template <typename T, typename U = typename std::make_unsigned<T>::type>
    inline typename std::enable_if<std::is_signed<T>::value, T>::type cpu2be(T value)
    {
        do_cpu2be(*reinterpret_cast<U*>(&value));
        return value;
    }

    void do_be2cpu(uint16_t&);
    void do_be2cpu(uint32_t&);
    void do_be2cpu(uint64_t&);

    inline void do_be2cpu(uint8_t&) {}

    template <typename T>
    inline typename std::enable_if<std::is_unsigned<T>::value, T>::type be2cpu(T value)
    {
        do_be2cpu(value);
        return value;
    }

    template <typename T, typename U = typename std::make_unsigned<T>::type>
    inline typename std::enable_if<std::is_signed<T>::value, T>::type be2cpu(T value)
    {
        do_be2cpu(*reinterpret_cast<U*>(&value));
        return value;
    }
}

#endif
