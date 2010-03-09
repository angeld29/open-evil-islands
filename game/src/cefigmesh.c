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
#include <stdbool.h>
#include <assert.h>

#include "celogging.h"
#include "cealloc.h"
#include "cefigmesh.h"

ce_figmesh* ce_figmesh_new(ce_figproto* figproto,
							const ce_complection* complection)
{
	ce_figmesh* figmesh = ce_alloc_zero(sizeof(ce_figmesh));
	if (NULL == figmesh) {
		ce_logging_error("figmesh: could not allocate memory");
		return NULL;
	}

	figmesh->figproto = ce_figproto_copy(figproto);
	ce_complection_copy(&figmesh->complection, complection);
	figmesh->ref_count = 1;

	return figmesh;
}

void ce_figmesh_del(ce_figmesh* figmesh)
{
	if (NULL != figmesh) {
		assert(figmesh->ref_count > 0);
		if (0 == --figmesh->ref_count) {
			ce_figproto_del(figmesh->figproto);
			ce_free(figmesh, sizeof(ce_figmesh));
		}
	}
}

ce_figmesh* ce_figmesh_copy(ce_figmesh* figmesh)
{
	++figmesh->ref_count;
	return figmesh;
}

