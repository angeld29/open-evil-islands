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
#include "cescenenode.h"
#include "cefighlp.h"
#include "cefigbone.h"

ce_figbone* ce_figbone_new(const ce_fignode* fignode,
							const ce_complection* complection,
							ce_figbone* parent)
{
	ce_figbone* figbone = ce_alloc(sizeof(ce_figbone));
	figbone->orientation = CE_QUAT_IDENTITY;
	figbone->anmstate = ce_anmstate_new();
	figbone->parent = parent;
	figbone->childs = ce_vector_new();

	ce_fighlp_get_bone(&figbone->position, fignode->figfile,
						fignode->bonfile, complection);

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_vector_push_back(figbone->childs, ce_figbone_new(
			fignode->childs->items[i], complection, figbone));
	}

	return figbone;
}

void ce_figbone_del(ce_figbone* figbone)
{
	if (NULL != figbone) {
		ce_vector_for_each(figbone->childs, ce_figbone_del);
		ce_vector_del(figbone->childs);
		ce_anmstate_del(figbone->anmstate);
		ce_free(figbone, sizeof(ce_figbone));
	}
}

void ce_figbone_advance(ce_figbone* figbone, float distance)
{
	ce_anmstate_advance(figbone->anmstate, distance);

	for (int i = 0; i < figbone->childs->count; ++i) {
		ce_figbone_advance(figbone->childs->items[i], distance);
	}
}

static void ce_figbone_update_transform(ce_figbone* figbone,
										ce_renderitem* renderitem)
{
	// update binding pose
	// note: translations from anmfile are not used...
	if (NULL == figbone->anmstate->anmfile) {
		figbone->orientation = CE_QUAT_IDENTITY;
	} else {
		ce_quat q1, q2;
		ce_quat_slerp(&figbone->orientation, figbone->anmstate->coef,
			ce_quat_init_array(&q1, figbone->anmstate->anmfile->rotations +
									(int)figbone->anmstate->prev_frame * 4),
			ce_quat_init_array(&q2, figbone->anmstate->anmfile->rotations +
									(int)figbone->anmstate->next_frame * 4));
	}

	// update bone pose
	if (NULL == figbone->parent) {
		// bone pose == binding pose
		figbone->bone_position = figbone->position;
		figbone->bone_orientation = figbone->orientation;
	} else {
		ce_vec3_rot(&figbone->bone_position,
					&figbone->position,
					&figbone->parent->bone_orientation);
		ce_vec3_add(&figbone->bone_position,
					&figbone->bone_position,
					&figbone->parent->bone_position);
		ce_quat_mul(&figbone->bone_orientation,
					&figbone->orientation,
					&figbone->parent->bone_orientation);
	}

	renderitem->position = figbone->bone_position;
	renderitem->orientation = figbone->bone_orientation;
}

static void ce_figbone_update_bounds(ce_figbone* figbone,
									ce_renderitem* renderitem)
{
	renderitem->bbox.aabb = renderitem->aabb;
	renderitem->bbox.axis = figbone->bone_orientation;

	ce_vec3_rot(&renderitem->bbox.aabb.origin,
				&renderitem->bbox.aabb.origin,
				&figbone->bone_orientation);
	ce_vec3_add(&renderitem->bbox.aabb.origin,
				&renderitem->bbox.aabb.origin,
				&figbone->bone_position);
}

void ce_figbone_update(ce_figbone* figbone,
						const ce_fignode* fignode,
						ce_vector* renderitems)
{
	ce_renderitem* renderitem = renderitems->items[fignode->index];

	ce_figbone_update_transform(figbone, renderitem);

	for (int i = 0; i < figbone->childs->count; ++i) {
		ce_figbone_update(figbone->childs->items[i],
							fignode->childs->items[i], renderitems);
	}

	ce_figbone_update_bounds(figbone, renderitem);

	ce_renderitem_update(renderitem, fignode->figfile, figbone->anmstate);
}

bool ce_figbone_play_animation(ce_figbone* figbone,
									const ce_fignode* fignode,
									const char* name)
{
	bool ok = ce_anmstate_play_animation(
		figbone->anmstate, fignode->anmfiles, name);
	for (int i = 0; i < figbone->childs->count; ++i) {
		ok = ce_figbone_play_animation(figbone->childs->items[i],
				fignode->childs->items[i], name) || ok;
	}
	return ok;
}

void ce_figbone_stop_animation(ce_figbone* figbone,
								const ce_fignode* fignode)
{
	ce_anmstate_stop_animation(figbone->anmstate);
	for (int i = 0; i < figbone->childs->count; ++i) {
		ce_figbone_stop_animation(figbone->childs->items[i],
									fignode->childs->items[i]);
	}
}
