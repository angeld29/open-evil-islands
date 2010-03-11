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

#include "celogging.h"
#include "cealloc.h"
#include "cestring.h"

ce_string* ce_string_new(void)
{
	return ce_string_new_reserved(16);
}

ce_string* ce_string_new_reserved(int capacity)
{
	ce_string* str = ce_alloc_zero(sizeof(ce_string));
	if (NULL == str) {
		ce_logging_error("string: could not allocate memory");
		return NULL;
	}

	ce_string_reserve(str, capacity);

	return str;
}

ce_string* ce_string_new_cstr(const char* cstr)
{
	ce_string* str = ce_string_new_reserved(strlen(cstr) + 1);
	if (NULL != str) {
		ce_string_assign(str, cstr);
	}
	return str;
}

ce_string* ce_string_new_cstr_n(const char* cstr, int n)
{
	ce_string* str = ce_string_new_reserved(n + 1);
	if (NULL != str) {
		ce_string_assign_n(str, cstr, n);
	}
	return str;
}

void ce_string_del(ce_string* str)
{
	if (NULL != str) {
		ce_free(str->str, str->capacity);
		ce_free(str, sizeof(ce_string));
	}
}

void ce_string_reserve(ce_string* str, int capacity)
{
	if (capacity > str->capacity) {
		char* cstr = ce_alloc(capacity);
		if (NULL != str->str) {
			strcpy(cstr, str->str);
			ce_free(str->str, str->capacity);
		} else {
			cstr[0] = '\0';
		}
		str->capacity = capacity;
		str->str = cstr;
	}
}

int ce_string_length(const ce_string* str)
{
	return str->length;
}

bool ce_string_empty(const ce_string* str)
{
	return 0 == str->length;
}

const char* ce_string_cstr(const ce_string* str)
{
	return str->str;
}

int ce_string_cmp(const ce_string* str1, const ce_string* str2)
{
	return strcmp(str1->str, str2->str);
}

int ce_string_cmp_cstr(const ce_string* str1, const char* str2)
{
	return strcmp(str1->str, str2);
}

ce_string* ce_string_dup(const ce_string* str)
{
	return ce_string_new_cstr(str->str);
}

ce_string* ce_string_dup_n(const ce_string* str, int n)
{
	return ce_string_new_cstr_n(str->str, n);
}

void ce_string_assign(ce_string* str, const char* cstr)
{
	int length = strlen(cstr);
	ce_string_reserve(str, length + 1);
	strcpy(str->str, cstr);
	str->length = length;
}

void ce_string_assign_n(ce_string* str, const char* cstr, int n)
{
	ce_string_reserve(str, n + 1);
	strncpy(str->str, cstr, n);
	str->str[n] = '\0';
	str->length = strlen(str->str);
}
