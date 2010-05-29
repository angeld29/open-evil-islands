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
#include <assert.h>

#include "cealloc.h"
#include "ceroot.h"

#include <ao/ao.h>
#include "cethread.h"
#include "cesoundsystem.h"

static ce_optparse* optparse;

static ce_thread* thread1;
static ce_thread* thread2;

static ce_soundinstance* instance1;
static ce_soundinstance* instance2;

static void ce_play(void* arg)
{
	ao_sample_format format;
	memset(&format, 0, sizeof(format));

	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_NATIVE;

	ao_device* device = ao_open_live(ao_default_driver_id(),
										&format, NULL);

	char buffer[4096];
	for (;;) {
		size_t size = ce_soundinstance_read(arg, buffer, sizeof(buffer));
		if (0 != size) {
			ao_play(device, buffer, size);
		} else {
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	ce_alloc_init();

	optparse = ce_root_create_optparse();

	ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any *.oga file in 'CE/Stream'");
	ce_optparse_add(optparse, "track2", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any *.oga file in 'CE/Stream'");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		ce_optparse_del(optparse);
		return EXIT_FAILURE;
	}

	const char* track;
	ce_optparse_get(optparse, "track", &track);

	const char* track2;
	ce_optparse_get(optparse, "track2", &track2);

	instance1 = ce_soundinstance_new_path(track);
	instance2 = ce_soundinstance_new_path(track2);

	thread1 = ce_thread_new(ce_play, instance1);
	thread2 = ce_thread_new(ce_play, instance2);

	ce_root_exec();

	ce_thread_wait(thread1);
	ce_thread_wait(thread2);

	ce_thread_del(thread1);
	ce_thread_del(thread2);

	//ce_soundmanager_play(ce_root.soundmanager, track);
	//ce_soundmanager_play(ce_root.soundmanager, track2);

	ce_soundinstance_del(instance1);
	ce_soundinstance_del(instance2);

	ce_optparse_del(optparse);

	return 0;
	//return ce_root_exec();
}
