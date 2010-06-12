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

ce_soundresource* ce_soundresource_new(ce_soundresource_vtable vtable, ce_memfile* memfile)
{
	size_t size = (*vtable.size_hint)(memfile);
	ce_memfile_rewind(memfile);

	ce_soundresource* soundresource = ce_alloc_zero(sizeof(ce_soundresource) + size);

	soundresource->memfile = memfile;
	soundresource->vtable = vtable;
	soundresource->size = size;

	if (!(*vtable.ctor)(soundresource)) {
		// do not take ownership if failed
		soundresource->memfile = NULL;
		ce_soundresource_del(soundresource);
		return NULL;
	}

	return soundresource;
}

ce_soundresource* ce_soundresource_new_builtin(ce_memfile* memfile)
{
	size_t index = ce_soundresource_find_builtin(memfile);
	if (CE_SOUNDRESOURCE_BUILTIN_COUNT != index) {
		return ce_soundresource_new(ce_soundresource_builtins[index], memfile);
	}
	return NULL;
}

void ce_soundresource_del(ce_soundresource* soundresource)
{
	if (NULL != soundresource) {
		if (NULL != soundresource->vtable.dtor) {
			(*soundresource->vtable.dtor)(soundresource);
		}
		ce_memfile_close(soundresource->memfile);
		ce_free(soundresource, sizeof(ce_soundresource) + soundresource->size);
	}
}

size_t ce_soundresource_find_builtin(ce_memfile* memfile)
{
	for (size_t i = 0; i < CE_SOUNDRESOURCE_BUILTIN_COUNT; ++i) {
		ce_memfile_rewind(memfile);
		if ((*ce_soundresource_builtins[i].test)(memfile)) {
			ce_memfile_rewind(memfile);
			return i;
		}
	}
	ce_memfile_rewind(memfile);
	return CE_SOUNDRESOURCE_BUILTIN_COUNT;
}

bool ce_soundresource_reset(ce_soundresource* soundresource)
{
	soundresource->time = 0.0f;
	ce_memfile_rewind(soundresource->memfile);
	return (*soundresource->vtable.reset)(soundresource);
}
