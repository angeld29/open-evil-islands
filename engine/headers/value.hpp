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

#ifndef CE_VALUE_HPP
#define CE_VALUE_HPP

#include "string.hpp"

namespace cursedearth
{
    typedef enum {
        CE_TYPE_VOID,
        CE_TYPE_BOOL,
        CE_TYPE_INT,
        CE_TYPE_FLOAT,
        CE_TYPE_STRING,
        CE_TYPE_COUNT
    } ce_type;

    typedef struct {
        ce_type type;
        union {
            bool b;
            int i;
            float f;
            ce_string* s;
        } value;
    } ce_value;

    ce_value* ce_value_new(ce_type type);
    void ce_value_del(ce_value* value);

    void ce_value_get(ce_value* value, void* arg);
    void ce_value_set(ce_value* value, const void* arg);

    inline bool ce_value_get_bool(ce_value* value)
    {
        bool arg;
        ce_value_get(value, &arg);
        return arg;
    }

    inline int ce_value_get_int(ce_value* value)
    {
        int arg;
        ce_value_get(value, &arg);
        return arg;
    }

    inline float ce_value_get_float(ce_value* value)
    {
        float arg;
        ce_value_get(value, &arg);
        return arg;
    }

    inline const char* ce_value_get_string(ce_value* value)
    {
        const char* arg = NULL;
        ce_value_get(value, &arg);
        return arg;
    }

    inline void ce_value_set_bool(ce_value* value, bool arg)
    {
        ce_value_set(value, &arg);
    }

    inline void ce_value_set_int(ce_value* value, int arg)
    {
        ce_value_set(value, &arg);
    }

    inline void ce_value_set_float(ce_value* value, float arg)
    {
        ce_value_set(value, &arg);
    }

    inline void ce_value_set_string(ce_value* value, const char* arg)
    {
        ce_value_set(value, arg);
    }
}

#endif
