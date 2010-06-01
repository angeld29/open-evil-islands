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
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundresource.h"

ce_soundresource* ce_soundresource_new(ce_soundresource_vtable vtable, ...)
{
	ce_soundresource* soundresource = ce_alloc_zero(sizeof(ce_soundresource) + vtable.size);
	soundresource->vtable = vtable;

	va_list args;
	va_start(args, vtable);

	if (!(*vtable.ctor)(soundresource, args)) {
		ce_soundresource_del(soundresource);
		soundresource = NULL;
	}

	va_end(args);

	return soundresource;
}

ce_soundresource* ce_soundresource_new_path(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		ce_logging_error("soundresource: could not open file '%s'", path);
		return NULL;
	}

	// TODO: loop and test
	ce_soundresource* soundresource = ce_soundresource_new(ce_soundresource_builtins[1], file);
	if (NULL == soundresource) {
		fclose(file);
	}

	return soundresource;
}

void ce_soundresource_del(ce_soundresource* soundresource)
{
	if (NULL != soundresource) {
		if (NULL != soundresource->vtable.dtor) {
			(*soundresource->vtable.dtor)(soundresource);
		}
		ce_free(soundresource, sizeof(ce_soundresource) + soundresource->vtable.size);
	}
}

size_t ce_soundresource_read(ce_soundresource* soundresource, void* data, size_t size)
{
	return (*soundresource->vtable.read)(soundresource, data, size);
}

bool ce_soundresource_rewind(ce_soundresource* soundresource)
{
	return (*soundresource->vtable.rewind)(soundresource);
}
