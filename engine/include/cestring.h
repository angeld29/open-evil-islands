/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	size_t capacity;
	size_t length;
	char* str;
} ce_string;

extern ce_string* ce_string_new(void);
extern ce_string* ce_string_new_reserved(size_t capacity);
extern ce_string* ce_string_new_str(const char* str);
extern ce_string* ce_string_new_str_n(const char* str, size_t n);
extern void ce_string_del(ce_string* string);

extern void ce_string_reserve(ce_string* string, size_t capacity);

static inline bool ce_string_empty(const ce_string* string)
{
	return 0 == string->length;
}

static inline ce_string* ce_string_dup(const ce_string* string)
{
	return ce_string_new_str(string->str);
}

static inline ce_string* ce_string_dup_n(const ce_string* string, size_t n)
{
	return ce_string_new_str_n(string->str, n);
}

extern void ce_string_assign(ce_string* string, const char* str);
extern void ce_string_assign_n(ce_string* string, const char* str, size_t n);
extern void ce_string_assign_va(ce_string* string, const char* fmt, va_list args);
extern void ce_string_assign_fmt(ce_string* string, const char* fmt, ...);

extern size_t ce_string_append(ce_string* string, const char* str);
extern size_t ce_string_append_n(ce_string* string, const char* str, size_t n);
extern size_t ce_string_append_va(ce_string* string, const char* fmt, va_list args);
extern size_t ce_string_append_fmt(ce_string* string, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* CE_STRING_H */
