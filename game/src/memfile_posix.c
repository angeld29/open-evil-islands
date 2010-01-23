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

#include <stdio.h>

#include "memfile.h"

memfile* memfile_open_data(void* data, size_t size, const char* mode)
{
	FILE* file = fmemopen(data, size, mode);
	if (NULL == file) {
		return NULL;
	}

	memfile* mem = memfile_open_callbacks(CEIO_CALLBACKS_FILE, file);
	if (NULL == mem) {
		fclose(file);
		return NULL;
	}

	return mem;
}
