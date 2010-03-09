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
#include "cefigentity.h"

ce_figentity* ce_figentity_new(ce_figmesh* figmesh)
{
	ce_figentity* figentity = ce_alloc_zero(sizeof(ce_figentity));
	if (NULL == figentity) {
		ce_logging_error("figentity: could not allocate memory");
		return NULL;
	}

	figentity->figmesh = ce_figmesh_copy(figmesh);

	return figentity;
}

void ce_figentity_del(ce_figentity* figentity)
{
	if (NULL != figentity) {
		ce_scenenode_del(figentity->scenenode);
		ce_figmesh_del(figentity->figmesh);
		ce_free(figentity, sizeof(ce_figentity));
	}
}
