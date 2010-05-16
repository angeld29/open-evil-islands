/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cevalue.h"

static void ce_value_proc_void(ce_value* value)
{
	ce_unused(value);
}

static void ce_value_proc_void_va(ce_value* value, va_list args)
{
	ce_unused(value);
	ce_unused(args);
}

static void ce_value_new_proc_string(ce_value* value)
{
	value->value.s = ce_string_new();
}

static void (*ce_value_new_procs[CE_TYPE_COUNT])(ce_value*) = {
	[CE_TYPE_VOID] = ce_value_proc_void,
	[CE_TYPE_BOOL] = ce_value_proc_void,
	[CE_TYPE_INT] = ce_value_proc_void,
	[CE_TYPE_FLOAT] = ce_value_proc_void,
	[CE_TYPE_STRING] = ce_value_new_proc_string,
};

static void ce_value_del_proc_string(ce_value* value)
{
	ce_string_del(value->value.s);
}

static void (*ce_value_del_procs[CE_TYPE_COUNT])(ce_value*) = {
	[CE_TYPE_VOID] = ce_value_proc_void,
	[CE_TYPE_BOOL] = ce_value_proc_void,
	[CE_TYPE_INT] = ce_value_proc_void,
	[CE_TYPE_FLOAT] = ce_value_proc_void,
	[CE_TYPE_STRING] = ce_value_del_proc_string,
};

static void ce_value_get_proc_bool(ce_value* value, va_list args)
{
	*va_arg(args, bool*) = value->value.b;
}

static void ce_value_get_proc_int(ce_value* value, va_list args)
{
	*va_arg(args, int*) = value->value.i;
}

static void ce_value_get_proc_float(ce_value* value, va_list args)
{
	*va_arg(args, float*) = value->value.f;
}

static void ce_value_get_proc_string(ce_value* value, va_list args)
{
	*va_arg(args, const char**) = value->value.s->str;
}

static void (*ce_value_get_procs[CE_TYPE_COUNT])(ce_value*, va_list) = {
	[CE_TYPE_VOID] = ce_value_proc_void_va,
	[CE_TYPE_BOOL] = ce_value_get_proc_bool,
	[CE_TYPE_INT] = ce_value_get_proc_int,
	[CE_TYPE_FLOAT] = ce_value_get_proc_float,
	[CE_TYPE_STRING] = ce_value_get_proc_string,
};

static void ce_value_set_proc_bool(ce_value* value, va_list args)
{
	value->value.b = va_arg(args, int);
}

static void ce_value_set_proc_int(ce_value* value, va_list args)
{
	value->value.i = va_arg(args, int);
}

static void ce_value_set_proc_float(ce_value* value, va_list args)
{
	value->value.f = va_arg(args, double);
}

static void ce_value_set_proc_string(ce_value* value, va_list args)
{
	ce_string_assign(value->value.s, va_arg(args, const char*));
}

static void (*ce_value_set_procs[CE_TYPE_COUNT])(ce_value*, va_list) = {
	[CE_TYPE_VOID] = ce_value_proc_void_va,
	[CE_TYPE_BOOL] = ce_value_set_proc_bool,
	[CE_TYPE_INT] = ce_value_set_proc_int,
	[CE_TYPE_FLOAT] = ce_value_set_proc_float,
	[CE_TYPE_STRING] = ce_value_set_proc_string,
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

void ce_value_get(ce_value* value, ...)
{
	va_list args;
	va_start(args, value);
	ce_value_get_va(value, args);
	va_end(args);
}

void ce_value_get_va(ce_value* value, va_list args)
{
	(*ce_value_get_procs[value->type])(value, args);
}

void ce_value_set(ce_value* value, ...)
{
	va_list args;
	va_start(args, value);
	ce_value_set_va(value, args);
	va_end(args);
}

void ce_value_set_va(ce_value* value, va_list args)
{
	(*ce_value_set_procs[value->type])(value, args);
}
