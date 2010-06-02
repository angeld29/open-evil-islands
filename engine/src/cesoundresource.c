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
	ce_soundresource* soundresource = ce_alloc_zero(sizeof(ce_soundresource) + vtable.size);

	soundresource->memfile = memfile;
	soundresource->vtable = vtable;

	if (!(*vtable.ctor)(soundresource)) {
		// do not take ownership if failed
		soundresource->memfile = NULL;
		ce_soundresource_del(soundresource);
		return NULL;
	}

	return soundresource;
}

ce_soundresource* ce_soundresource_new_builtin_path(const char* path)
{
	ce_memfile* memfile = ce_memfile_open_path(path);
	if (NULL == memfile) {
		ce_logging_error("soundresource: could not load sound: '%s'", path);
		return NULL;
	}

	for (size_t i = 0; i < CE_SOUNDRESOURCE_BUILTIN_COUNT; ++i) {
		bool ok = true; // TODO: test
		// TODO: rewind
		if (ok) {
			ce_soundresource* soundresource =
				ce_soundresource_new(ce_soundresource_builtins[i], memfile);

			if (NULL == soundresource)  {
				ce_logging_error("soundresource: could not load sound: '%s'", path);
				ce_memfile_close(memfile);
				return NULL;
			}

			return soundresource;
		}
	}

	ce_logging_error("soundresource: no appropriate sound decoder found: '%s'", path);
	ce_memfile_close(memfile);
	return NULL;
}

void ce_soundresource_del(ce_soundresource* soundresource)
{
	if (NULL != soundresource) {
		if (NULL != soundresource->vtable.dtor) {
			(*soundresource->vtable.dtor)(soundresource);
		}
		ce_memfile_close(soundresource->memfile);
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
