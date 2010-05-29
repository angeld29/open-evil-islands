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
#include <assert.h>

#if defined(__MINGW32__) && defined(__STRICT_ANSI__)
// HACK: add fseeko64 function prototype
#include <sys/types.h>
int __cdecl __MINGW_NOTHROW fseeko64(FILE*, off64_t, int);
#endif

#include <vorbis/vorbisfile.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "cesoundinstance.h"

struct ce_soundinstance {
	OggVorbis_File vf;
	int bigendianp;
	int bitstream;
};

ce_soundinstance* ce_soundinstance_new_path(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		ce_logging_error("soundinstance: could not open file '%s'", path);
		return NULL;
	}

	ce_soundinstance* soundinstance = ce_alloc_zero(sizeof(ce_soundinstance));
	soundinstance->bigendianp = ce_is_big_endian();

	ce_unused(OV_CALLBACKS_NOCLOSE);
	ce_unused(OV_CALLBACKS_STREAMONLY);
	ce_unused(OV_CALLBACKS_STREAMONLY_NOCLOSE);

	if (0 != ov_open_callbacks(file, &soundinstance->vf, NULL, 0, OV_CALLBACKS_DEFAULT)) {
		ce_logging_error("soundinstance: '%s' does not appear to be an ogg bitstream", path);
		ce_soundinstance_del(soundinstance);
		fclose(file);
		return NULL;
	}

	vorbis_info* info = ov_info(&soundinstance->vf, -1);
	if (NULL != info) {
		ce_logging_write("soundinstance: '%s' is %d channel, %ld Hz, %ld bps",
			path, info->channels, info->rate, ov_bitrate(&soundinstance->vf, -1));
	}

	return soundinstance;
}

void ce_soundinstance_del(ce_soundinstance* soundinstance)
{
	if (NULL != soundinstance) {
		ov_clear(&soundinstance->vf);
		ce_free(soundinstance, sizeof(ce_soundinstance));
	}
}

size_t ce_soundinstance_read(ce_soundinstance* soundinstance, void* buffer, size_t size)
{
	for (;;) {
		long code = ov_read(&soundinstance->vf, buffer, size,
			soundinstance->bigendianp, 2, 1, &soundinstance->bitstream);
		if (code >= 0) {
			return code;
		}
		ce_logging_warning("soundinstance: error in the stream");
	}
}
