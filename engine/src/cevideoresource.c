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

#include "cealloc.h"
#include "celogging.h"
#include "cevideoresource.h"

extern const size_t CE_VIDEORESOURCE_BUILTIN_COUNT;
extern const ce_videoresource_vtable ce_videoresource_builtins[];

ce_videoresource* ce_videoresource_new(ce_memfile* memfile)
{
	size_t index;
	for (index = 0; index < CE_VIDEORESOURCE_BUILTIN_COUNT; ++index) {
		ce_memfile_rewind(memfile);
		if ((*ce_videoresource_builtins[index].test)(memfile)) {
			break;
		}
	}

	ce_memfile_rewind(memfile);
	if (CE_VIDEORESOURCE_BUILTIN_COUNT == index) {
		return NULL;
	}

	size_t size = (*ce_videoresource_builtins[index].size_hint)(memfile);
	ce_memfile_rewind(memfile);

	ce_videoresource* videoresource = ce_alloc_zero(sizeof(ce_videoresource) + size);

	videoresource->memfile = memfile;
	videoresource->vtable = ce_videoresource_builtins[index];
	videoresource->size = size;

	if (!(*videoresource->vtable.ctor)(videoresource)) {
		// do not take ownership if failed
		videoresource->memfile = NULL;
		ce_videoresource_del(videoresource);
		return NULL;
	}

	return videoresource;
}

void ce_videoresource_del(ce_videoresource* videoresource)
{
	if (NULL != videoresource) {
		if (NULL != videoresource->vtable.dtor) {
			(*videoresource->vtable.dtor)(videoresource);
		}
		ce_memfile_close(videoresource->memfile);
		ce_free(videoresource, sizeof(ce_videoresource) + videoresource->size);
	}
}

bool ce_videoresource_reset(ce_videoresource* videoresource)
{
	videoresource->time = 0.0f;
	videoresource->frame_index = 0;
	ce_memfile_rewind(videoresource->memfile);
	return (*videoresource->vtable.reset)(videoresource);
}
