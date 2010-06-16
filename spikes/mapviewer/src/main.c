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

static ce_optparse* optparse;

static ce_inputsupply* inputsupply;
static ce_inputevent* anmfps_inc_event;
static ce_inputevent* anmfps_dec_event;

static float message_timeout;
static ce_color message_color;

static void clean()
{
	ce_inputsupply_del(inputsupply);
	ce_optparse_del(optparse);
}

static void state_changed(void* listener, int state)
{
	ce_unused(listener);

	if (CE_SCENEMNG_STATE_READY == state) {
		const char* zone;
		ce_optparse_get(optparse, "zone", &zone);
		ce_scenemng_load_mpr(ce_root.scenemng, zone);
		ce_scenemng_load_mob(ce_root.scenemng, zone);
		ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
	}

	if (CE_SCENEMNG_STATE_PLAYING == state) {
		// play random animations
		srand(time(NULL));
		for (size_t i = 0; i < ce_root.scenemng->figentities->count; ++i) {
			ce_figentity* figentity = ce_root.scenemng->figentities->items[i];
			int anm_count = ce_figentity_get_animation_count(figentity);
			if (anm_count > 0) {
				const char* name = ce_figentity_get_animation_name(figentity,
														rand() % anm_count);
				ce_figentity_play_animation(figentity, name);
			}
		}

		if (NULL != ce_root.scenemng->terrain) {
			ce_vec3 position;
			ce_camera_set_position(ce_root.scenemng->camera, ce_vec3_init(&position,
				0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f));
			ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f),
															ce_deg2rad(30.0f));
		}
	}
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_inputsupply_advance(inputsupply, elapsed);

	float anmfps = ce_root.anmfps;

	if (anmfps_inc_event->triggered) anmfps += 1.0f;
	if (anmfps_dec_event->triggered) anmfps -= 1.0f;

	if (message_timeout > 0.0f) {
		message_timeout -= elapsed;
	}

	if (anmfps != ce_root.anmfps) {
		message_timeout = 3.0f;
	}

	message_color.a = ce_fclamp(message_timeout, 0.0f, 1.0f);
	ce_root.anmfps = ce_fclamp(anmfps, 1.0f, 50.0f);
}

static void render(void* listener)
{
	ce_unused(listener);
	if (message_timeout > 0.0f) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "Animation FPS: %d", (int)ce_root.anmfps);
		ce_font_render(ce_root.scenemng->font, (ce_root.scenemng->viewport->width -
			ce_font_get_width(ce_root.scenemng->font, buffer)) / 2,
			1 * (ce_root.scenemng->viewport->height -
			ce_font_get_height(ce_root.scenemng->font)) / 5,
			&message_color, buffer);
	}
}

int main(int argc, char* argv[])
{
	ce_alloc_init();
	atexit(clean);

	optparse = ce_root_create_optparse();

	ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"Cursed Earth: Map Viewer", "This program is part of Cursed Earth "
		"spikes\nMap Viewer - explore Evil Islands zones with creatures");

	ce_optparse_add(optparse, "zone", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any ZONE.mpr file in 'EI/Maps'");

	ce_optparse_add_control(optparse, "+/-", "change animation FPS");

	if (!ce_root_init(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	const char* ei_path;
	ce_optparse_get(optparse, "ei_path", &ei_path);

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

	ce_root.scenemng->listener = (ce_scenemng_listener)
		{.state_changed = state_changed, .advance = advance, .render = render};

	message_color = CE_COLOR_CORNFLOWER;

	inputsupply = ce_inputsupply_new(ce_root.renderwindow->inputcontext);
	anmfps_inc_event = ce_inputsupply_repeat(inputsupply,
						ce_inputsupply_button(inputsupply,
							CE_KB_ADD), CE_INPUT_DEFAULT_DELAY, 10);
	anmfps_dec_event = ce_inputsupply_repeat(inputsupply,
						ce_inputsupply_button(inputsupply,
							CE_KB_SUBTRACT), CE_INPUT_DEFAULT_DELAY, 10);

	return ce_root_exec();
}
