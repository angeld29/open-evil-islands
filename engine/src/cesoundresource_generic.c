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
#include "cesoundresource.h"

typedef struct {
	OggVorbis_File vf;
	int bitstream;
} ce_soundresource_vorbis;

static bool ce_soundresource_vorbis_ctor(ce_soundresource* soundresource, va_list args)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;

	ce_unused(OV_CALLBACKS_NOCLOSE);
	ce_unused(OV_CALLBACKS_STREAMONLY);
	ce_unused(OV_CALLBACKS_STREAMONLY_NOCLOSE);

	if (0 != ov_open_callbacks(va_arg(args, FILE*), &vorbisresource->vf, NULL, 0, OV_CALLBACKS_DEFAULT)) {
		ce_logging_error("soundresource: input does not appear to be an ogg bitstream");
		return false;
	}

	vorbis_info* info = ov_info(&vorbisresource->vf, -1);
	if (NULL == info) {
		ce_logging_error("soundresource: could not get stream info");
		return false;
	}

	// a vorbis file has no particular number of bits per sample,
	// so use words, see also ov_read
	soundresource->bps = 16;
	soundresource->rate = info->rate;
	soundresource->channels = info->channels;

	return true;
}

static void ce_soundresource_vorbis_dtor(ce_soundresource* soundresource)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;
	ov_clear(&vorbisresource->vf);
}

static size_t ce_soundresource_vorbis_read(ce_soundresource* soundresource, void* buffer, size_t size)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;

	for (;;) {
		long code = ov_read(&vorbisresource->vf, buffer, size,
					ce_is_big_endian(), 2, 1, &vorbisresource->bitstream);
		if (code >= 0) {
			return code;
		}
		ce_logging_warning("soundresource: error in the stream");
	}
}

#ifdef CE_NONFREE
// TODO: implement mad
#endif

const ce_soundresource_vtable ce_soundresource_builtins[] = {
	{sizeof(ce_soundresource_vorbis), ce_soundresource_vorbis_ctor,
	ce_soundresource_vorbis_dtor, ce_soundresource_vorbis_read, NULL},
#ifdef CE_NONFREE
	// TODO: implement mad
#endif
};

const size_t CE_SOUNDRESOURCE_BUILTIN_COUNT = sizeof(ce_soundresource_builtins) /
											sizeof(ce_soundresource_builtins[0]);
