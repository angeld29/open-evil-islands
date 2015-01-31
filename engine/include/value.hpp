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

#ifndef CE_VALUE_H
#define CE_VALUE_H

#include "cestring.h"

#ifdef __cplusplus
extern "C" {
#endif

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

extern ce_value* ce_value_new(ce_type type);
extern void ce_value_del(ce_value* value);

extern void ce_value_get(ce_value* value, void* arg);
extern void ce_value_set(ce_value* value, const void* arg);

static inline bool ce_value_get_bool(ce_value* value)
{
    bool arg;
    ce_value_get(value, &arg);
    return arg;
}

static inline int ce_value_get_int(ce_value* value)
{
    int arg;
    ce_value_get(value, &arg);
    return arg;
}

static inline float ce_value_get_float(ce_value* value)
{
    float arg;
    ce_value_get(value, &arg);
    return arg;
}

static inline const char* ce_value_get_string(ce_value* value)
{
    const char* arg = NULL;
    ce_value_get(value, &arg);
    return arg;
}

static inline void ce_value_set_bool(ce_value* value, bool arg)
{
    ce_value_set(value, &arg);
}

static inline void ce_value_set_int(ce_value* value, int arg)
{
    ce_value_set(value, &arg);
}

static inline void ce_value_set_float(ce_value* value, float arg)
{
    ce_value_set(value, &arg);
}

static inline void ce_value_set_string(ce_value* value, const char* arg)
{
    ce_value_set(value, arg);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_VALUE_H */
