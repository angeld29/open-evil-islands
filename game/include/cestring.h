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

#ifndef CE_STRING_H
#define CE_STRING_H

#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	int capacity;
	int length;
	char* str;
} ce_string;

extern ce_string* ce_string_new(void);
extern ce_string* ce_string_new_reserved(int capacity);
extern ce_string* ce_string_new_str(const char* str);
extern ce_string* ce_string_new_str_n(const char* str, int n);
extern void ce_string_del(ce_string* string);

extern void ce_string_reserve(ce_string* string, int capacity);

extern bool ce_string_empty(const ce_string* string);

extern ce_string* ce_string_dup(const ce_string* string);
extern ce_string* ce_string_dup_n(const ce_string* string, int n);

extern void ce_string_assign(ce_string* string, const char* str);
extern void ce_string_assign_n(ce_string* string, const char* str, int n);
extern void ce_string_assign_v(ce_string* string, const char* fmt, va_list args);
extern void ce_string_assign_f(ce_string* string, const char* fmt, ...);

extern int ce_string_append(ce_string* string, const char* str);
extern int ce_string_append_n(ce_string* string, const char* str, int n);
extern int ce_string_append_v(ce_string* string, const char* fmt, va_list args);
extern int ce_string_append_f(ce_string* string, const char* fmt, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_STRING_H */
