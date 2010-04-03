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

#include <assert.h>

#include "cealloc.h"
#include "cefighlp.h"
#include "cefigrenderitem.h"
#include "cefigmesh.h"

static void ce_figmesh_create_renderitems(ce_figmesh* figmesh,
										const ce_fignode* fignode)
{
	ce_renderitem* renderitem =
		ce_figrenderitem_new(fignode, &figmesh->complection);

	ce_fighlp_get_aabb(&renderitem->aabb, fignode->figfile,
											&figmesh->complection);

	ce_vector_push_back(figmesh->renderitems, renderitem);

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_figmesh_create_renderitems(figmesh, fignode->childs->items[i]);
	}
}

ce_figmesh* ce_figmesh_new(ce_figproto* figproto,
							const ce_complection* complection)
{
	ce_figmesh* figmesh = ce_alloc(sizeof(ce_figmesh));
	figmesh->figproto = ce_figproto_add_ref(figproto);
	figmesh->complection = *complection;
	figmesh->renderitems = ce_vector_new();
	figmesh->ref_count = 1;
	ce_figmesh_create_renderitems(figmesh, figmesh->figproto->fignode);
	return figmesh;
}

void ce_figmesh_del(ce_figmesh* figmesh)
{
	if (NULL != figmesh) {
		assert(figmesh->ref_count > 0);
		if (0 == --figmesh->ref_count) {
			for (int i = 0; i < figmesh->renderitems->count; ++i) {
				ce_renderitem_del(figmesh->renderitems->items[i]);
			}
			ce_vector_del(figmesh->renderitems);
			ce_figproto_del(figmesh->figproto);
			ce_free(figmesh, sizeof(ce_figmesh));
		}
	}
}

ce_figmesh* ce_figmesh_add_ref(ce_figmesh* figmesh)
{
	++figmesh->ref_count;
	return figmesh;
}
