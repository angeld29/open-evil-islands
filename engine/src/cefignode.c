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

#include "cestr.h"
#include "cealloc.h"
#include "cefighlp.h"
#include "cefignode.h"

ce_fignode* ce_fignode_new(ce_resfile* mod_resfile,
							ce_resfile* bon_resfile,
							ce_vector* anm_resfiles,
							ce_lnkfile* lnkfile)
{
	ce_fignode* fignode = ce_alloc(sizeof(ce_fignode));
	fignode->name = ce_string_dup(lnkfile->links[lnkfile->link_index].child_name);
	fignode->index = lnkfile->link_index++;
	fignode->figfile = ce_figfile_open(mod_resfile, fignode->name->str);
	fignode->bonfile = ce_bonfile_open(bon_resfile, fignode->name->str);
	fignode->anmfiles = ce_vector_new();
	fignode->material = ce_fighlp_create_material(fignode->figfile);
	fignode->rendergroup = NULL;
	fignode->childs = ce_vector_new();

	for (int i = 0; i < anm_resfiles->count; ++i) {
		ce_resfile* anm_resfile = anm_resfiles->items[i];
		int anm_index = ce_resfile_node_index(anm_resfile, fignode->name->str);
		if (-1 != anm_index) {
			ce_vector_push_back(fignode->anmfiles,
				ce_anmfile_open(anm_resfile, anm_index));
		} // else ok, there is no animation for this node
	}

	while (lnkfile->link_index < lnkfile->link_count &&
			0 == ce_strcasecmp(fignode->name->str,
				lnkfile->links[lnkfile->link_index].parent_name->str)) {
		ce_vector_push_back(fignode->childs,
			ce_fignode_new(mod_resfile, bon_resfile, anm_resfiles, lnkfile));
	}

	return fignode;
}

void ce_fignode_del(ce_fignode* fignode)
{
	if (NULL != fignode) {
		ce_vector_for_each(fignode->childs, ce_fignode_del);
		ce_vector_for_each(fignode->anmfiles, ce_anmfile_close);
		ce_vector_del(fignode->childs);
		ce_material_del(fignode->material);
		ce_vector_del(fignode->anmfiles);
		ce_bonfile_close(fignode->bonfile);
		ce_figfile_close(fignode->figfile);
		ce_string_del(fignode->name);
		ce_free(fignode, sizeof(ce_fignode));
	}
}

void ce_fignode_accept_renderqueue_cascade(ce_fignode* fignode,
											ce_renderqueue* renderqueue)
{
	fignode->rendergroup = ce_renderqueue_get(renderqueue,
		fignode->figfile->material_group, fignode->material);

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_fignode_accept_renderqueue_cascade(fignode->childs->items[i],
															renderqueue);
	}
}
