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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cealloc.h"
#include "cestring.h"

ce_string* ce_string_new(void)
{
	return ce_string_new_reserved(16);
}

ce_string* ce_string_new_reserved(size_t capacity)
{
	ce_string* string = ce_alloc_zero(sizeof(ce_string));
	ce_string_reserve(string, capacity);
	return string;
}

ce_string* ce_string_new_str(const char* str)
{
	ce_string* string = ce_string_new_reserved(strlen(str) + 1);
	ce_string_assign(string, str);
	return string;
}

ce_string* ce_string_new_str_n(const char* str, size_t n)
{
	ce_string* string = ce_string_new_reserved(n + 1);
	ce_string_assign_n(string, str, n);
	return string;
}

void ce_string_del(ce_string* string)
{
	if (NULL != string) {
		ce_free(string->str, string->capacity);
		ce_free(string, sizeof(ce_string));
	}
}

void ce_string_reserve(ce_string* string, size_t capacity)
{
	if (capacity > string->capacity) {
		string->str = ce_realloc(string->str, string->capacity, capacity);
		string->capacity = capacity;
	}
}

void ce_string_assign(ce_string* string, const char* str)
{
	size_t length = strlen(str);
	ce_string_reserve(string, length + 1);
	strcpy(string->str, str);
	string->length = length;
}

void ce_string_assign_n(ce_string* string, const char* str, size_t n)
{
	ce_string_reserve(string, n + 1);
	strncpy(string->str, str, n);
	string->str[n] = '\0';
	string->length = strlen(string->str);
}

void ce_string_assign_va(ce_string* string, const char* fmt, va_list args)
{
	for (size_t length = 128, limit = 1024; length <= limit; length <<= 1) {
		char buffer[length];
		int code = vsnprintf(buffer, sizeof(buffer), fmt, args);
		if (code >= 0 && (size_t)code < sizeof(buffer)) {
			ce_string_assign(string, buffer);
			break;
		}
	}
}

void ce_string_assign_fmt(ce_string* string, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ce_string_assign_va(string, fmt, args);
	va_end(args);
}

size_t ce_string_append(ce_string* string, const char* str)
{
	size_t length = strlen(str);
	ce_string_reserve(string, string->length + length + 1);
	strcat(string->str, str);
	string->length += length;
	return length;
}

size_t ce_string_append_n(ce_string* string, const char* str, size_t n)
{
	size_t old_length = string->length;
	ce_string_reserve(string, string->length + n + 1);
	strncat(string->str, str, n);
	string->length = strlen(string->str);
	return string->length - old_length;
}

size_t ce_string_append_va(ce_string* string, const char* fmt, va_list args)
{
	for (size_t length = 128, limit = 1024; length <= limit; length <<= 1) {
		char buffer[length];
		int code = vsnprintf(buffer, sizeof(buffer), fmt, args);
		if (code >= 0 && (size_t)code < sizeof(buffer)) {
			return ce_string_append(string, buffer);
		}
	}
	return 0;
}

size_t ce_string_append_fmt(ce_string* string, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t count = ce_string_append_va(string, fmt, args);
	va_end(args);
	return count;
}
