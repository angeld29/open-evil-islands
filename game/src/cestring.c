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

#include <string.h>

#include "cealloc.h"
#include "cestring.h"

ce_string* ce_string_new(void)
{
	return ce_string_new_reserved(16);
}

ce_string* ce_string_new_reserved(int capacity)
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

ce_string* ce_string_new_str_n(const char* str, int n)
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

void ce_string_reserve(ce_string* string, int capacity)
{
	if (capacity > string->capacity) {
		char* str = ce_alloc(capacity);
		strncpy(str, string->str, string->length);
		str[string->length] = '\0';
		ce_free(string->str, string->capacity);
		string->capacity = capacity;
		string->str = str;
	}
}

bool ce_string_empty(const ce_string* string)
{
	return 0 == string->length;
}

ce_string* ce_string_dup(const ce_string* string)
{
	return ce_string_new_str(string->str);
}

ce_string* ce_string_dup_n(const ce_string* string, int n)
{
	return ce_string_new_str_n(string->str, n);
}

void ce_string_assign(ce_string* string, const char* str)
{
	int length = strlen(str);
	ce_string_reserve(string, length + 1);
	strcpy(string->str, str);
	string->length = length;
}

void ce_string_assign_n(ce_string* string, const char* str, int n)
{
	ce_string_reserve(string, n + 1);
	strncpy(string->str, str, n);
	string->str[n] = '\0';
	string->length = strlen(string->str);
}
