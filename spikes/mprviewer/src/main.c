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
#include <assert.h>

#include <argtable2.h>

#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

static void usage(const char* progname, void* argtable[])
{
	fprintf(stderr, "Cursed Earth is an open source, "
		"cross-platform port of Evil Islands\n"
		"Copyright (C) 2009-2010 Yanis Kurganov\n\n");

	fprintf(stderr, "This program is part of Cursed Earth spikes\n"
		"MPR Viewer - explore clean Evil Islands zones\n\n");

	fprintf(stderr, "usage: %s", progname);
	arg_print_syntax(stderr, argtable, "\n");
	arg_print_glossary_gnu(stderr, argtable);

	void* ctrtable[] = {
		arg_rem("keyboard arrows", "move camera"),
		arg_rem("mouse motion", "rotate camera"),
		arg_rem("mouse wheel", "zoom camera"),
		arg_rem("b", "show/hide bounding boxes"),
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

	if (!ce_root_init(ei_path->sval[0])) {
		return EXIT_FAILURE;
	}

	// FIXME: move to root
	if (0 != terrain_tiling->count) {
		ce_root.terrain_tiling = true;
	}

	if (0 != jobs->count) {
		ce_root.thread_count = jobs->ival[0];
	}

	ce_logging_write("root: using up to %d threads", ce_root.thread_count);
	ce_logging_write("root: terrain tiling %s",
		ce_root.terrain_tiling ? "enabled" : "disabled");

	if (NULL == ce_scenemng_create_terrain(ce_root.scenemng, zone->sval[0],
							&CE_VEC3_ZERO, &CE_QUAT_IDENTITY, NULL)) {
		return EXIT_FAILURE;
	}

	ce_vec3 position;
	ce_vec3_init(&position, 0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f);

	ce_camera_set_position(ce_root.scenemng->camera, &position);
	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f), ce_deg2rad(30.0f));

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	ce_root_exec();

	return EXIT_SUCCESS;
}
