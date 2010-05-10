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
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include <argtable2.h>

#include "cegl.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

#ifndef CE_SPIKE_VERSION_MAJOR
#define CE_SPIKE_VERSION_MAJOR 0
#endif
#ifndef CE_SPIKE_VERSION_MINOR
#define CE_SPIKE_VERSION_MINOR 0
#endif
#ifndef CE_SPIKE_VERSION_PATCH
#define CE_SPIKE_VERSION_PATCH 0
#endif

//static ce_input_event_supply* es;
//static ce_input_event* anmfps_inc_event;
//static ce_input_event* anmfps_dec_event;

//static float anmfps_limit = 0.1f;
//static float anmfps_inc_counter;
//static float anmfps_dec_counter;

/*static void idle(void)
{
	//es = ce_input_event_supply_new(ce_root.renderwindow->input_context);
	//anmfps_inc_event = ce_input_event_supply_button(es, CE_KB_ADD);
	//anmfps_dec_event = ce_input_event_supply_button(es, CE_KB_SUBTRACT);

	anmfps_inc_counter += elapsed;
	anmfps_dec_counter += elapsed;

	if (ce_input_event_triggered(anmfps_inc_event) &&
			anmfps_inc_counter >= anmfps_limit) {
		scenemng->anmfps += 1.0f;
		anmfps_inc_counter = 0.0f;
	}

	if (ce_input_event_triggered(anmfps_dec_event) &&
			anmfps_dec_counter >= anmfps_limit) {
		scenemng->anmfps -= 1.0f;
		anmfps_dec_counter = 0.0f;
	}

	scenemng->anmfps = ce_fclamp(scenemng->anmfps, 1.0f, 50.0f);
}*/

static void usage(const char* progname, void* argtable[])
{
	fprintf(stderr, "Cursed Earth is an open source, "
		"cross-platform port of Evil Islands\n"
		"Copyright (C) 2009-2010 Yanis Kurganov\n\n");

	fprintf(stderr, "This program is part of Cursed Earth spikes\n"
		"Map Viewer - explore Evil Islands zones with creatures\n\n");

	fprintf(stderr, "usage: %s", progname);
	arg_print_syntax(stderr, argtable, "\n");
	arg_print_glossary_gnu(stderr, argtable);

	void* ctrtable[] = {
		arg_rem("keyboard arrows", "move camera"),
		arg_rem("mouse motion", "rotate camera"),
		arg_rem("mouse wheel", "zoom camera"),
		arg_rem("b", "show/hide bounding boxes"),
		arg_rem("+/-", "change animation FPS"),
		arg_end(0)
	};

	fprintf(stderr, "controls:\n");
	arg_print_glossary_gnu(stderr, ctrtable);
	arg_freetable(ctrtable, sizeof(ctrtable) / sizeof(ctrtable[0]));
}

int main(int argc, char* argv[])
{
	struct arg_lit* help = arg_lit0("h", "help", "display this help and exit");
	struct arg_lit* version = arg_lit0("v", "version",
		"display version information and exit");
	struct arg_str* ei_path = arg_str0("b", "ei-path", "DIRECTORY",
		"path to EI directory (current directory by default)");
	struct arg_lit* full_screen = arg_lit0("f", "full-screen",
		"start program in full screen mode");
	struct arg_lit* terrain_tiling = arg_lit0("t", "terrain-tiling",
		"enable terrain tiling; very slow, but reduce usage of "
		"video memory and disk space; use it on old video cards");
	struct arg_int* jobs = arg_int0("j", "jobs", "N",
		"allow N jobs at once; if this option is not specified, the "
		"value N will be detected automatically depending on the number "
		"of CPUs you have (or the number of cores your CPU have)");
	struct arg_str* zone = arg_str1(NULL, NULL,
		"ZONE", "any ZONE.mpr file in 'ei-path/Maps'");
	struct arg_end* end = arg_end(3);

	void* argtable[] = {
		help, version, ei_path, full_screen, terrain_tiling, jobs, zone, end
	};

	ei_path->sval[0] = ".";

	int argerror_count = arg_parse(argc, argv, argtable);

	if (0 != help->count) {
		usage(argv[0], argtable);
		return EXIT_SUCCESS;
	}

	if (0 != version->count) {
		fprintf(stderr, "%d.%d.%d\n", CE_SPIKE_VERSION_MAJOR,
										CE_SPIKE_VERSION_MINOR,
										CE_SPIKE_VERSION_PATCH);
		return EXIT_SUCCESS;
	}

	if (0 != argerror_count) {
		usage(argv[0], argtable);
		arg_print_errors(stderr, end, argv[0]);
		return EXIT_FAILURE;
	}

	ce_root_init(ei_path->sval[0]);

	if (0 != terrain_tiling->count) {
		ce_root.scenemng->terrain_tiling = true;
	}

	if (0 != jobs->count) {
		ce_root.scenemng->thread_count = jobs->ival[0];
	}

	ce_logging_write("scenemng: using up to %d threads", ce_root.scenemng->thread_count);
	ce_logging_write("scenemng: terrain tiling %s",
		ce_root.scenemng->terrain_tiling ? "enabled" : "disabled");

	if (NULL == ce_scenemng_create_terrain(ce_root.scenemng, zone->sval[0],
					&CE_VEC3_ZERO, &CE_QUAT_IDENTITY, NULL)) {
		return EXIT_FAILURE;
	}

	char path[strlen(ei_path->sval[0]) + strlen(zone->sval[0]) + 32];
	snprintf(path, sizeof(path), "%s/Maps/%s.mob", ei_path->sval[0], zone->sval[0]);

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

	snprintf(path, sizeof(path), "%s/Camera/%s.cam",
		ei_path->sval[0], zone->sval[0]/*"mainmenu"*/);

	ce_vec3 position;
	ce_vec3_init(&position, 0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f);

	ce_camera_set_position(ce_root.scenemng->camera, &position);
	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f), ce_deg2rad(30.0f));

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

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	ce_root_exec();
	ce_root_term();

	return EXIT_SUCCESS;
}
