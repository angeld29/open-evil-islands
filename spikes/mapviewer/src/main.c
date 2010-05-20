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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "celib.h"
#include "cemath.h"
#include "cealloc.h"
#include "ceroot.h"

static ce_input_event_supply* input_supply;
static ce_input_event* anmfps_inc_event;
static ce_input_event* anmfps_dec_event;

static float anmfps_limit = 0.1f;
static float anmfps_inc_counter;
static float anmfps_dec_counter;

static void destroy_input()
{
	ce_input_event_supply_del(input_supply);
}

static void create_input()
{
	input_supply = ce_input_event_supply_new(ce_root.renderwindow->input_context);
	anmfps_inc_event = ce_input_event_supply_button(input_supply, CE_KB_ADD);
	anmfps_dec_event = ce_input_event_supply_button(input_supply, CE_KB_SUBTRACT);
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_input_event_supply_advance(input_supply, elapsed);

	anmfps_inc_counter += elapsed;
	anmfps_dec_counter += elapsed;

	if (anmfps_inc_event->triggered && anmfps_inc_counter >= anmfps_limit) {
		ce_root.anmfps += 1.0f;
		anmfps_inc_counter = 0.0f;
	}

	if (anmfps_dec_event->triggered && anmfps_dec_counter >= anmfps_limit) {
		ce_root.anmfps -= 1.0f;
		anmfps_dec_counter = 0.0f;
	}

	ce_root.anmfps = ce_fclamp(ce_root.anmfps, 1.0f, 50.0f);
}

static ce_scenemng_listener scenemng_listener = { .onadvance = advance };

int main(int argc, char* argv[])
{
	ce_alloc_init();

	ce_optparse* optparse = ce_root_create_optparse();

	ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"Cursed Earth: Map Viewer", "This program is part of Cursed Earth "
		"spikes\nMap Viewer - explore Evil Islands zones with creatures");

	ce_optparse_add(optparse, "zone", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any ZONE.mpr file in 'EI/Maps'");

	ce_optparse_add_control(optparse, "+/-", "change animation FPS");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		ce_optparse_del(optparse);
		return EXIT_FAILURE;
	}

	const char *ei_path, *zone;

	ce_optparse_get(optparse, "ei_path", &ei_path);
	ce_optparse_get(optparse, "zone", &zone);

	if (NULL == ce_scenemng_create_terrain(ce_root.scenemng, zone,
							&CE_VEC3_ZERO, &CE_QUAT_IDENTITY, NULL)) {
		ce_optparse_del(optparse);
		return EXIT_FAILURE;
	}

	char path[strlen(ei_path) + strlen(zone) + 32];
	snprintf(path, sizeof(path), "%s/Maps/%s.mob", ei_path, zone);

	ce_mobfile* mobfile = ce_mobfile_open(path);
	if (NULL != mobfile) {
		ce_scenemng_load_mobfile(ce_root.scenemng, mobfile);
	}
	ce_mobfile_close(mobfile);

	srand(time(NULL));

	// play random animations
	for (int i = 0; i < ce_root.scenemng->figmng->figentities->count; ++i) {
		ce_figentity* figentity = ce_root.scenemng->figmng->figentities->items[i];
		int anm_count = ce_figentity_get_animation_count(figentity);
		if (anm_count > 0) {
			const char* name = ce_figentity_get_animation_name(figentity,
														rand() % anm_count);
			ce_figentity_play_animation(figentity, name);
		}
	}

	ce_vec3 position;
	ce_camera_set_position(ce_root.scenemng->camera, ce_vec3_init(&position,
		0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f));

	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f),
													ce_deg2rad(30.0f));

	//snprintf(path, sizeof(path), "%s/Camera/%s.cam",
	//	ei_path->sval[0], zone->sval[0]/*"mainmenu"*/);

	/*FILE* file = NULL;//fopen(path, "rb");
	if (NULL != file) {
		for (;;) {
			uint32_t v1, v2;

			if (0 == fread(&v1, 4, 1, file)) {
				break;
			}

			fread(&v2, 4, 1, file);

			printf("%u %u\n", v1, v2);

			float f[4];
			fread(f, sizeof(float), 3, file);
			printf("%f %f %f\n", f[0], f[1], f[2]);

			ce_vec3 position;
			ce_vec3_init_array(&position, f);
			ce_fswap(&position.z, &position.y);
			position.y = scenemng->terrain->mprfile->max_y;
			position.z = -position.z;

			ce_camera_set_position(scenemng->camera, &position);

			fread(f, sizeof(float), 4, file);
			printf("%f %f %f %f\n", f[0], f[1], f[2], f[3]);

			ce_quat orientation, temp, temp2;//, temp3;
			ce_quat_init_array(&temp, f);

			ce_quat_init_polar(&temp2, ce_deg2rad(90), &CE_VEC3_UNIT_X);
			//ce_quat_mul(&temp3, &temp2, &temp);

			ce_quat_mul(&orientation, &temp2, &temp);

			//ce_quat_init_polar(&temp2, ce_deg2rad(180), &CE_VEC3_UNIT_Y);
			//ce_quat_mul(&orientation, &temp2, &temp3);

			ce_quat_conj(&orientation, &orientation);

			ce_camera_set_orientation(scenemng->camera, &orientation);
		}
		fclose(file);
	}*/

	create_input();
	atexit(destroy_input);

	ce_scenemng_add_listener(ce_root.scenemng, &scenemng_listener);

	ce_optparse_del(optparse);

	return ce_root_exec();
}
