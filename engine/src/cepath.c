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

#include "cestr.h"
#include "cepath.h"

char* ce_path_join_va(char* path, size_t size, va_list args)
{
	for (const char* tail = va_arg(args, const char*);
			NULL != tail; tail = va_arg(args, const char*)) {
		size_t length = strlen(path);
		if (0 != length && '/' != path[length - 1] && '\\' != path[length - 1]) {
			ce_strlcat(path, (const char[]){CE_PATH_SEP, '\0'}, size);
		}
		ce_strlcat(path, tail, size);
	}
	return path;
}

char* ce_path_join(char* path, size_t size, ...)
{
	va_list args;
	va_start(args, size);
	ce_path_join_va(path, size, args);
	va_end(args);
	return path;
}

char* ce_path_join_clear(char* path, size_t size, ...)
{
	va_list args;
	va_start(args, size);
	memset(path, 0, size);
	ce_path_join_va(path, size, args);
	va_end(args);
	return path;
}

char* ce_path_normpath(char* path)
{
	for (char* sep = strpbrk(path, "/\\");
			NULL != sep; sep = strpbrk(sep + 1, "/\\")) {
		*sep = CE_PATH_SEP;
	}
	return path;
}
