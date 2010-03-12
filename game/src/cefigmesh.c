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
#include "cefighlp.h"
#include "cefigrenderitem.h"
#include "cefigmesh.h"

static void ce_figmesh_node_del(ce_figmesh_node* node)
{
	if (NULL != node) {
		if (NULL != node->child_nodes) {
			for (int i = 0, n = ce_vector_count(node->child_nodes); i < n; ++i) {
				ce_figmesh_node_del(ce_vector_at(node->child_nodes, i));
			}
			ce_vector_del(node->child_nodes);
		}
		ce_renderitem_del(node->renderitem);
		ce_free(node, sizeof(ce_figmesh_node));
	}
}

static ce_figmesh_node* ce_figmesh_node_new(const ce_fignode* proto_node,
											const ce_complection* complection)
{
	ce_figmesh_node* node = ce_alloc_zero(sizeof(ce_figmesh_node));
	if (NULL == node) {
		ce_logging_error("figmesh node: could not allocate memory");
		return NULL;
	}

	node->proto_node = proto_node;

	ce_fighlp_get_bone(&node->bone, proto_node->figfile,
						proto_node->bonfile, complection);

	if (NULL == (node->renderitem =
					ce_figrenderitem_new(proto_node->figfile, complection,
										false/*proto_node->has_morphing*/)) ||
			NULL == (node->child_nodes = ce_vector_new())) {
		ce_figmesh_node_del(node);
		return NULL;
	}

	for (int i = 0, n = ce_vector_count(proto_node->childs); i < n; ++i) {
		ce_figmesh_node* child_node = ce_figmesh_node_new(
			ce_vector_at(proto_node->childs, i), complection);
		if (NULL == child_node) {
			ce_figmesh_node_del(node);
			return NULL;
		}
		ce_vector_push_back(node->child_nodes, child_node);
	}

	return node;
}

ce_figmesh* ce_figmesh_new(ce_figproto* figproto,
							const ce_complection* complection)
{
	ce_figmesh* figmesh = ce_alloc_zero(sizeof(ce_figmesh));
	if (NULL == figmesh) {
		ce_logging_error("figmesh: could not allocate memory");
		return NULL;
	}

	figmesh->figproto = figproto;
	figmesh->ref_count = 1;

	ce_complection_copy(&figmesh->complection, complection);

	figmesh->root_node = ce_figmesh_node_new(figproto->fignode, complection);

	return figmesh;
}

void ce_figmesh_del(ce_figmesh* figmesh)
{
	if (NULL != figmesh) {
		assert(figmesh->ref_count > 0);
		if (0 == --figmesh->ref_count) {
			ce_figmesh_node_del(figmesh->root_node);
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

