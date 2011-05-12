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

#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "celogging.h"
#include "cefighlp.h"

ce_aabb* ce_fighlp_get_aabb(ce_aabb* aabb, const ce_figfile* figfile,
							const ce_complection* complection)
{
	if (0 == figfile->vertex_count) {
		// I found some strange figure files!
		// all bounds in these ones are NAN or INF...
		return ce_aabb_init_zero(aabb);
	}

	aabb->radius = figfile->value_callback(figfile->radius, 1, complection);

	ce_vec3_init(&aabb->origin,
		figfile->value_callback(figfile->center + 0, 3, complection),
		figfile->value_callback(figfile->center + 1, 3, complection),
		figfile->value_callback(figfile->center + 2, 3, complection));

	ce_vec3 min, max;
	ce_vec3_init(&min,
		figfile->value_callback(figfile->min + 0, 3, complection),
		figfile->value_callback(figfile->min + 1, 3, complection),
		figfile->value_callback(figfile->min + 2, 3, complection));
	ce_vec3_init(&max,
		figfile->value_callback(figfile->max + 0, 3, complection),
		figfile->value_callback(figfile->max + 1, 3, complection),
		figfile->value_callback(figfile->max + 2, 3, complection));

	// add extra space to guarantee full coating of the object
	const float extra = 0.1f;

	ce_vec3_sub(&aabb->extents, &max, &min);
	ce_vec3_scale(&aabb->extents, 0.5f + extra, &aabb->extents);

	return aabb;
}

ce_sphere* ce_fighlp_get_sphere(ce_sphere* sphere, const ce_figfile* figfile,
								const ce_complection* complection)
{
	ce_vec3 center;
	return ce_sphere_init(sphere, ce_vec3_init(&center,
		figfile->value_callback(figfile->center + 0, 3, complection),
		figfile->value_callback(figfile->center + 1, 3, complection),
		figfile->value_callback(figfile->center + 2, 3, complection)),
		figfile->value_callback(figfile->radius, 1, complection));
}

float* ce_fighlp_get_vertex(float* array, const ce_figfile* figfile,
							int index, const ce_complection* complection)
{
	// see doc/formats/figfile.txt for details!
	size_t i = 3 * figfile->value_count * 4 * (index / 4) + index % 4;
	size_t j = 3 * figfile->value_count * 4 / 3;
	array[0] = figfile->value_callback(
				figfile->vertices + i + 0 * j, 4, complection);
	array[1] = figfile->value_callback(
				figfile->vertices + i + 1 * j, 4, complection);
	array[2] = figfile->value_callback(
				figfile->vertices + i + 2 * j, 4, complection);
	return array;
}

float* ce_fighlp_get_normal(float* array, const ce_figfile* figfile, int index)
{
	size_t i = 4 * 4 * (index / 4) + index % 4;
	float inv_w = 1.0f / figfile->normals[i + 12];
	array[0] = figfile->normals[i + 0] * inv_w;
	array[1] = figfile->normals[i + 4] * inv_w;
	array[2] = figfile->normals[i + 8] * inv_w;
	return array;
}

ce_vec3* ce_fighlp_get_bone(ce_vec3* position,
							const ce_figfile* figfile,
							const ce_bonfile* bonfile,
							const ce_complection* complection)
{
	return ce_vec3_init(position,
		figfile->value_callback(bonfile->bone + 0, 3, complection),
		figfile->value_callback(bonfile->bone + 1, 3, complection),
		figfile->value_callback(bonfile->bone + 2, 3, complection));
}

ce_material* ce_fighlp_create_material(const ce_figfile* figfile)
{
	ce_material* material = ce_material_new();
	material->mode = CE_MATERIAL_MODE_REPLACE;

	switch (figfile->material_group) {
	case 17: // unmoli, unmosp
		material->alpha_test = true;
		break;
	case 18: // nafltr59, stst82, unmosk
		material->alpha_test = true;
		break;
	case 19: // unhuma, unhufe, unmozo0, unmozo1
		break;
	case 21: // nafltr82, unmosu, unmocy, unmodg, unmogo
		material->alpha_test = true;
		break;
	case 22: // nast10, stbuho62, stwa1
		break;
	case 23: // unmoel1, unmosh
		material->alpha_test = true;
		break;
	case 25: // unmowi
		material->blend = true;
		break;
	default:
		assert(false);
	}

	return material;
}

bool ce_fighlp_is_node_tess_blacklisted(const ce_string* fignodename, const ce_string* figprotoname)
{
	/// here`s a lot of hardcode! but I don`t see any other way
	bool is_blacklisted = false;
	const char * figprotonamestr = figprotoname->str;
	const char * fignodenamestr = fignodename->str;
	/// debug output to get figproto name and fignode name
	if (strncmp(figprotonamestr,"un",2) && strncmp(figprotonamestr,"na",2))
		printf("%s %s\n", figprotonamestr, fignodenamestr);

	/// figproto name first and by nodename second
	if (!strncmp(figprotonamestr,"st",2)) {						// stationary? human made stuff
		is_blacklisted = true;									// blacklisted by default
		if (!strncmp(&figprotonamestr[2],"br17",4)) {				// whitelisted illithyd bridge
			if (strncmp(fignodenamestr,"col1",4) &&				// except side borders
				strncmp(fignodenamestr,"col2",4) &&
				strncmp(fignodenamestr,"base1",5))					// and moving part of bridge
			{
				is_blacklisted = false;
			}
		}
		else if (!strncmp(&figprotonamestr[2],"bu",2)) {			// building
			if (!strncmp(&figprotonamestr[4],"ho",2)) {			// house - some whitelisted buildings
				if (!strncmp(&figprotonamestr[6],"11",2) ||		// big hadagan tent
					!strncmp(&figprotonamestr[6],"3",2) ||			// ogre`s hut
					!strncmp(&figprotonamestr[6],"39",2) ||		// hadagan metal bell crown
					!strncmp(&figprotonamestr[6],"46",2) ||		// necromancer`s house
					!strncmp(&figprotonamestr[6],"55",2))			// damagedhadagan metal bell crown
				{
					is_blacklisted = false;
				}
				else if (!strncmp(&figprotonamestr[6],"60",2))	{	// illithyd building
					if (strncmp(fignodenamestr,"base",4))			// except base
						is_blacklisted = false;
				}
			}
		}
		if (!strncmp(&figprotonamestr[2],"st",2)) {				// statues? not-building human made stuff
			if (!strncmp(&figprotonamestr[4],"002",3) ||			// a lot of interpolation-frendly objects
				!strncmp(&figprotonamestr[4],"1",3) ||
				!strncmp(&figprotonamestr[4],"10",3) ||
				!strncmp(&figprotonamestr[4],"102",3) ||
				!strncmp(&figprotonamestr[4],"106",3) ||
				!strncmp(&figprotonamestr[4],"107",3) ||
				!strncmp(&figprotonamestr[4],"11",3) ||
				!strncmp(&figprotonamestr[4],"113",3) ||
				!strncmp(&figprotonamestr[4],"114",3) ||
				!strncmp(&figprotonamestr[4],"116",3) ||
				!strncmp(&figprotonamestr[4],"119",3) ||
				!strncmp(&figprotonamestr[4],"120",3) ||	//?
				!strncmp(&figprotonamestr[4],"121",3) ||
				!strncmp(&figprotonamestr[4],"122",3) ||	//?
				!strncmp(&figprotonamestr[4],"123",3) ||	//?
				!strncmp(&figprotonamestr[4],"124",3) ||
				!strncmp(&figprotonamestr[4],"127",3) ||
				!strncmp(&figprotonamestr[4],"131",3) ||
				!strncmp(&figprotonamestr[4],"134",3) ||
				!strncmp(&figprotonamestr[4],"135",3) ||
				!strncmp(&figprotonamestr[4],"136",3) ||
				!strncmp(&figprotonamestr[4],"137",3) ||
				!strncmp(&figprotonamestr[4],"138",3) ||
				!strncmp(&figprotonamestr[4],"143",3) ||
				!strncmp(&figprotonamestr[4],"15",3) ||
				!strncmp(&figprotonamestr[4],"154",3) ||
				!strncmp(&figprotonamestr[4],"155",3) ||
				!strncmp(&figprotonamestr[4],"156",3) ||
				!strncmp(&figprotonamestr[4],"157",3) ||
				!strncmp(&figprotonamestr[4],"16",3) ||
				!strncmp(&figprotonamestr[4],"160",3) ||
				!strncmp(&figprotonamestr[4],"17",3) ||
				!strncmp(&figprotonamestr[4],"18",3) ||	//?
				!strncmp(&figprotonamestr[4],"19",3) ||
				!strncmp(&figprotonamestr[4],"2",3) ||		//?
				!strncmp(&figprotonamestr[4],"20",3) ||
				!strncmp(&figprotonamestr[4],"28",3) ||
				!strncmp(&figprotonamestr[4],"30",3) ||
				!strncmp(&figprotonamestr[4],"31",3) ||
				!strncmp(&figprotonamestr[4],"36",3) ||
				!strncmp(&figprotonamestr[4],"37",3) ||
				!strncmp(&figprotonamestr[4],"64",3) ||
				!strncmp(&figprotonamestr[4],"67",3) ||
				!strncmp(&figprotonamestr[4],"70",3) ||	//?
				!strncmp(&figprotonamestr[4],"73",3) ||	//illithid furniture*
				!strncmp(&figprotonamestr[4],"74",3) ||	//
				!strncmp(&figprotonamestr[4],"75",3) ||	//
				!strncmp(&figprotonamestr[4],"76",3) ||	//*illithid furniture
				!strncmp(&figprotonamestr[4],"80",3) ||	//?
				!strncmp(&figprotonamestr[4],"81",3) ||	//?
				!strncmp(&figprotonamestr[4],"83",3) ||	//?
				!strncmp(&figprotonamestr[4],"84",3) ||	//?
				!strncmp(&figprotonamestr[4],"88",3) ||
				!strncmp(&figprotonamestr[4],"89",3) ||
				!strncmp(&figprotonamestr[4],"90",3) ||
				!strncmp(&figprotonamestr[4],"91",3) ||
				!strncmp(&figprotonamestr[4],"93",3))
			{
				printf("WHITELISTED STATUE!!!!!!!\n");
				is_blacklisted = false;
			}
			else if (!strncmp(&figprotonamestr[4],"001",3))	// big stone jun face
			{
				if (strncmp(fignodenamestr,"box",3))			// except box
					is_blacklisted = false;
			}
			else if (!strncmp(&figprotonamestr[4],"12",3))		// dragon on box
			{
				if (!strncmp(fignodenamestr,"dragon",6))		// dragon only
					is_blacklisted = false;
			}
			else if (!strncmp(&figprotonamestr[4],"56",3))		//sack with money
			{
				if (!strncmp(fignodenamestr,"sack",2))			// sack only
					is_blacklisted = false;
			}
			else if (!strncmp(&figprotonamestr[4],"87",3))		// goblin head on pike
			{
				if (!strncmp(fignodenamestr,"hd",2))			// head only
					is_blacklisted = false;
			}
			else if (!strncmp(&figprotonamestr[4],"112",3))	// eagle on scull with mantled wings
			{
				if (strncmp(fignodenamestr,"eag1",4))			// except eagle
					is_blacklisted = false;
			}
			else if (!strncmp(&figprotonamestr[4],"141",3))	// whitch`s kettle
			{
				if (!strncmp(fignodenamestr,"kettle",6))		// kettle only
					is_blacklisted = false;
			}
		}
		if (!strncmp(&figprotonamestr[2],"wa",2)) {			// walls
			if (!strncmp(&figprotonamestr[4],"6",2) ||			// wood gnomes? walls
				!strncmp(&figprotonamestr[4],"7",2) ||
				!strncmp(&figprotonamestr[4],"8",2) ||
				!strncmp(&figprotonamestr[4],"9",2))
			{
				is_blacklisted = false;
			}
			if (!strncmp(&figprotonamestr[4],"10",2) ||		// hadagan walls
				!strncmp(&figprotonamestr[4],"11",2) ||
				!strncmp(&figprotonamestr[4],"14",2))
			{
				if (!strncmp(fignodenamestr,"crown",5))		// crowns only
					is_blacklisted = false;
			}

		}
		if (!strncmp(&figprotonamestr[2],"we",2)) 				// wells - interpolate
			is_blacklisted = false;
	}
	else if (!strncmp(figprotonamestr,"nafltr",6)) {				// nature flowers? trees
		if (!strncmp(&figprotonamestr[6],"2",1) ||			//2*	// stabs and logs
			!strncmp(&figprotonamestr[6],"70",2) ||
			!strncmp(&figprotonamestr[6],"74",2) ||
			!strncmp(&figprotonamestr[6],"83",2) ||
			!strncmp(&figprotonamestr[6],"84",2) ||
			!strncmp(&figprotonamestr[6],"85",2) ||
			!strncmp(&figprotonamestr[6],"86",2))
		{
			is_blacklisted = true;
		}
	}
	else if (!strncmp(figprotonamestr,"un",2)) {					// units
		if (!strncmp(&figprotonamestr[2],"mo",2)) {				// monsters
			if (!strncmp(&figprotonamestr[4],"ba",2))				// banshee
				is_blacklisted = true;
			else if (!strncmp(&figprotonamestr[4],"co",2))			// menu column
				is_blacklisted = true;
			else if (!strncmp(&figprotonamestr[4],"go",2)) {		// goblin or golem
				if (!strncmp(&figprotonamestr[6],"\0",1)) {		// goblin
					if (!strncmp(fignodenamestr,"rh3.pike00",10))	// goblin`s pike
						is_blacklisted = true;
				}
				else											// golems
					is_blacklisted = true;
			}
			else if (!strncmp(&figprotonamestr[4],"og1",3)) {		// ogre with wood club - bone looks fine
				if (!strncmp(fignodenamestr,"club",4))			// ogre`s club
					is_blacklisted = true;
			}
			else if (!strncmp(&figprotonamestr[4],"li",2)) {		// lizardmen
				if (!strncmp(fignodenamestr,"rh3.trident",11))		// lizardmen`s trident
					is_blacklisted = true;
			}
			else if (!strncmp(&figprotonamestr[4],"ri",2)) {		// tka-rick
				if (!strncmp(fignodenamestr,"rh3.staff00",11))		// lizardmen`s trident
					is_blacklisted = true;
			}
			else if (!strncmp(&figprotonamestr[4],"wi",2)) {		// fire stones
				is_blacklisted = true;
			}
		}
	}
	return is_blacklisted;
}
