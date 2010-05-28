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

#include <vorbis/vorbisfile.h>
#include <ao/ao.h>

#include "cebyteorder.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesound.h"

struct ce_sound {
	OggVorbis_File vf;
	int bigendianp;
	int bitstream;
};

ce_sound* ce_sound_new_file(FILE* file)
{
	ce_sound* sound = ce_alloc_zero(sizeof(ce_sound));
	sound->bigendianp = ce_is_big_endian();

	if (0 != ov_open_callbacks(file, &sound->vf, NULL, 0, OV_CALLBACKS_DEFAULT)) {
		ce_logging_error("sound: input does not appear to be an ogg bitstream");
		ce_sound_del(sound);
		return NULL;
	}

	vorbis_info* info = ov_info(&sound->vf, -1);
	if (NULL != info) {
		ce_logging_write("sound: bitstream is %d channel, %ld Hz, %ld bps, %ld bps",
			info->channels, info->rate, info->bitrate_nominal, ov_bitrate(&sound->vf, -1));
	}

	return sound;
}

void ce_sound_del(ce_sound* sound)
{
	if (NULL != sound) {
		ov_clear(&sound->vf);
		ce_free(sound, sizeof(ce_sound));
	}
}

void ce_sound_read(ce_sound* sound)
{
	bool eof = false;
	char pcm[512];

	while (!eof) {
		long code = ov_read(&sound->vf, pcm, sizeof(pcm),
			sound->bigendianp, 2, 1, &sound->bitstream);
		if (0 == code) {
			eof = true;
		} else if (code < 0) {
			ce_logging_warning("sound: error in the stream");
		} else {
			// TODO: use pcm and code = actual number of bytes read
		}
	}
}

struct ce_soundmng {
	ao_device* device;
	ao_sample_format format;
};

ce_soundmng* ce_soundmng_new(void)
{
	ao_initialize();

	ce_soundmng* soundmng = ce_alloc_zero(sizeof(ce_soundmng));
	soundmng->format.bits = 16;
	soundmng->format.channels = 2;
	soundmng->format.rate = 44100;
	soundmng->format.byte_format = AO_FMT_NATIVE;
	soundmng->device = ao_open_live(ao_default_driver_id(),
									&soundmng->format, NULL);

	if (NULL == soundmng->device) {
		ce_logging_error("soundmng: error opening device");
		ce_soundmng_del(soundmng);
		return NULL;
	}

	ao_info* info = ao_driver_info(soundmng->device->driver_id);
	if (NULL != info) {
		ce_logging_write("soundmng: using %s", info->short_name);
		ce_logging_write("soundmng: %s", info->name);
		ce_logging_write("soundmng: %s", info->comment);
	} else {
		ce_logging_warning("soundmng: could not get driver info");
	}

	return soundmng;
}

void ce_soundmng_del(ce_soundmng* soundmng)
{
	if (NULL != soundmng) {
		if (NULL != soundmng->device) {
			ao_close(soundmng->device);
		}
		ce_free(soundmng, sizeof(ce_soundmng));
	}

	ao_shutdown();
}

void ce_soundmng_play(ce_soundmng* soundmng)
{
	//ao_play(device, buffer, size);
}
