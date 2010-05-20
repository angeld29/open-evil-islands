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

#include "cemath.h"
#include "cealloc.h"
#include "ceroot.h"

int main(int argc, char* argv[])
{
	ce_alloc_init();

	ce_optparse* optparse = ce_root_create_optparse();

	ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"Cursed Earth: MPR Viewer", "This program is part of Cursed "
		"Earth spikes\nMPR Viewer - explore clean Evil Islands zones");

	ce_optparse_add(optparse, "zone", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any ZONE.mpr file in 'EI/Maps'");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		ce_optparse_del(optparse);
		return EXIT_FAILURE;
	}

	const char* zone;
	ce_optparse_get(optparse, "zone", &zone);

	if (NULL == ce_scenemng_create_terrain(ce_root.scenemng, zone,
							&CE_VEC3_ZERO, &CE_QUAT_IDENTITY, NULL)) {
		return EXIT_FAILURE;
	}

	ce_vec3 position;
	ce_camera_set_position(ce_root.scenemng->camera, ce_vec3_init(&position,
		0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f));

	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f),
													ce_deg2rad(30.0f));

	ce_optparse_del(optparse);

	return ce_root_exec();
}
