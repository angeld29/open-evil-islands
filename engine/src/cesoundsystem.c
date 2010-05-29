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
#include <assert.h>

#include <ao/ao.h>

#include "celogging.h"
#include "cesoundsystem.h"

static struct {
	bool inited;
	ao_device* device;
	ao_device* device2;
	int in_use;
	ao_sample_format format;
} ce_soundsystem;

static void ce_soundsystem_term(void)
{
	assert(ce_soundsystem.inited && "the sound subsystem has not yet been inited");
	ce_soundsystem.inited = false;

	if (NULL != ce_soundsystem.device2) {
		ao_close(ce_soundsystem.device2);
	}

	if (NULL != ce_soundsystem.device) {
		ao_close(ce_soundsystem.device);
	}

	ao_shutdown();
}

bool ce_soundsystem_init(void)
{
	assert(!ce_soundsystem.inited && "the sound subsystem has already been inited");
	ce_soundsystem.inited = true;

	atexit(ce_soundsystem_term);

	ao_initialize();

	ce_soundsystem.format.bits = 16;
	ce_soundsystem.format.channels = 2;
	ce_soundsystem.format.rate = 44100;
	ce_soundsystem.format.byte_format = AO_FMT_NATIVE;

	ce_soundsystem.device = ao_open_live(ao_default_driver_id(),
										&ce_soundsystem.format, NULL);
	if (NULL == ce_soundsystem.device) {
		ce_logging_error("soundsystem: error opening device");
		return false;
	}

	ce_soundsystem.device2 = ao_open_live(ao_default_driver_id(),
										&ce_soundsystem.format, NULL);
	if (NULL == ce_soundsystem.device) {
		ce_logging_error("soundsystem: error opening device");
		return false;
	}

	ao_info* info = ao_driver_info(ao_default_driver_id());
	if (NULL != info) {
		ce_logging_write("soundsystem: using %s (%s)", info->short_name, info->name);
		ce_logging_write("soundsystem: %s", info->comment);
	}

	return true;
}

bool ce_soundsystem_play(const void* buffer, size_t size)
{
	assert(ce_soundsystem.inited && "the sound subsystem has not yet been inited");

	ao_device* device;
	if (0 == ce_soundsystem.in_use) {
		device = ce_soundsystem.device;
		//ce_soundsystem.in_use = 1;
	} else {
		device = ce_soundsystem.device2;
		//ce_soundsystem.in_use = 0;
	}

	return NULL == device ? false :
		0 != ao_play(device, (void*)buffer, size);
}
