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

#ifndef CE_STRING_HPP
#define CE_STRING_HPP

#include <cstddef>
#include <cstdarg>

namespace cursedearth
{
    typedef struct {
        size_t capacity;
        size_t length;
        char* str;
    } ce_string;

    ce_string* ce_string_new(void);
    ce_string* ce_string_new_reserved(size_t capacity);
    ce_string* ce_string_new_str(const char* str);
    ce_string* ce_string_new_str_n(const char* str, size_t n);
    void ce_string_del(ce_string* string);

    void ce_string_reserve(ce_string* string, size_t capacity);

    inline bool ce_string_empty(const ce_string* string)
    {
        return 0 == string->length;
    }

    inline ce_string* ce_string_dup(const ce_string* string)
    {
        return ce_string_new_str(string->str);
    }

    inline ce_string* ce_string_dup_n(const ce_string* string, size_t n)
    {
        return ce_string_new_str_n(string->str, n);
    }

    void ce_string_assign(ce_string* string, const char* str);
    void ce_string_assign_n(ce_string* string, const char* str, size_t n);
    void ce_string_assign_va(ce_string* string, const char* fmt, va_list args);
    void ce_string_assign_fmt(ce_string* string, const char* fmt, ...);

    size_t ce_string_append(ce_string* string, const char* str);
    size_t ce_string_append_n(ce_string* string, const char* str, size_t n);
    size_t ce_string_append_va(ce_string* string, const char* fmt, va_list args);
    size_t ce_string_append_fmt(ce_string* string, const char* fmt, ...);
}

#endif
