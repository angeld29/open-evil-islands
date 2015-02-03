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

#include "alloc.hpp"
#include "value.hpp"

static void ce_value_void(ce_value*)
{
}

static void ce_value_void_arg(ce_value*, void*)
{
}

static void ce_value_void_arg_const(ce_value*, const void*)
{
}

static void (*ce_value_new_procs[CE_TYPE_COUNT])(ce_value*) = {
    [CE_TYPE_VOID] = ce_value_void,
    [CE_TYPE_BOOL] = ce_value_void,
    [CE_TYPE_INT] = ce_value_void,
    [CE_TYPE_FLOAT] = ce_value_void,
    [CE_TYPE_STRING] = ce_value_void,
};

static void ce_value_del_string(ce_value* value)
{
    ce_string_del(value->value.s);
}

static void (*ce_value_del_procs[CE_TYPE_COUNT])(ce_value*) = {
    [CE_TYPE_VOID] = ce_value_void,
    [CE_TYPE_BOOL] = ce_value_void,
    [CE_TYPE_INT] = ce_value_void,
    [CE_TYPE_FLOAT] = ce_value_void,
    [CE_TYPE_STRING] = ce_value_del_string,
};

static void ce_value_get_bool_proc(ce_value* value, void* arg)
{
    *(bool*)arg = value->value.b;
}

static void ce_value_get_int_proc(ce_value* value, void* arg)
{
    *(int*)arg = value->value.i;
}

static void ce_value_get_float_proc(ce_value* value, void* arg)
{
    *(float*)arg = value->value.f;
}

static void ce_value_get_string_proc(ce_value* value, void* arg)
{
    *(const char**)arg = NULL != value->value.s ? value->value.s->str : NULL;
}

static void (*ce_value_get_procs[CE_TYPE_COUNT])(ce_value*, void*) = {
    [CE_TYPE_VOID] = ce_value_void_arg,
    [CE_TYPE_BOOL] = ce_value_get_bool_proc,
    [CE_TYPE_INT] = ce_value_get_int_proc,
    [CE_TYPE_FLOAT] = ce_value_get_float_proc,
    [CE_TYPE_STRING] = ce_value_get_string_proc,
};

static void ce_value_set_bool_proc(ce_value* value, const void* arg)
{
    if (NULL != arg) value->value.b = *(const bool*)arg;
}

static void ce_value_set_int_proc(ce_value* value, const void* arg)
{
    if (NULL != arg) value->value.i = *(const int*)arg;
}

static void ce_value_set_float_proc(ce_value* value, const void* arg)
{
    if (NULL != arg) value->value.f = *(const float*)arg;
}

static void ce_value_set_string_proc(ce_value* value, const void* arg)
{
    if (NULL != arg) {
        if (NULL == value->value.s) {
            value->value.s = ce_string_new_str(arg);
        } else {
            ce_string_assign(value->value.s, arg);
        }
    }
}

static void (*ce_value_set_procs[CE_TYPE_COUNT])(ce_value*, const void*) = {
    [CE_TYPE_VOID] = ce_value_void_arg_const,
    [CE_TYPE_BOOL] = ce_value_set_bool_proc,
    [CE_TYPE_INT] = ce_value_set_int_proc,
    [CE_TYPE_FLOAT] = ce_value_set_float_proc,
    [CE_TYPE_STRING] = ce_value_set_string_proc,
};

ce_value* ce_value_new(ce_type type)
{
    ce_value* value = ce_alloc_zero(sizeof(ce_value));
    value->type = type;
    (*ce_value_new_procs[type])(value);
    return value;
}

void ce_value_del(ce_value* value)
{
    if (NULL != value) {
        (*ce_value_del_procs[value->type])(value);
        ce_free(value, sizeof(ce_value));
    }
}

void ce_value_get(ce_value* value, void* arg)
{
    (*ce_value_get_procs[value->type])(value, arg);
}

void ce_value_set(ce_value* value, const void* arg)
{
    (*ce_value_set_procs[value->type])(value, arg);
}
