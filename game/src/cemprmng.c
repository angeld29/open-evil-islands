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

#include <stdio.h>

#include "celogging.h"
#include "cealloc.h"
#include "ceresfile.h"
#include "cemprmng.h"

ce_mprmng* ce_mprmng_new(const char* maps_path)
{
	ce_mprmng* mprmng = ce_alloc_zero(sizeof(ce_mprmng));
	if (NULL == mprmng) {
		ce_logging_error("mprmng: could not allocate memory");
		return NULL;
	}

	if (NULL == (mprmng->maps_path = ce_string_new_cstr(maps_path))) {
		ce_mprmng_del(mprmng);
		return NULL;
	}

	ce_logging_write("mprmng: using path: '%s'", maps_path);

	return mprmng;
}

void ce_mprmng_del(ce_mprmng* mprmng)
{
	if (NULL != mprmng) {
		ce_string_del(mprmng->maps_path);
		ce_free(mprmng, sizeof(ce_mprmng));
	}
}

ce_mprfile* ce_mprmng_open_mprfile(ce_mprmng* mprmng, const char* zone_name)
{
	char path[512];
	snprintf(path, sizeof(path), "%s/%s.mpr",
			ce_string_cstr(mprmng->maps_path), zone_name);

	ce_resfile* resfile = ce_resfile_open_file(path);
	if (NULL == resfile) {
		ce_logging_error("mprmng: could not open map: '%s'", zone_name);
		return NULL;
	}

	ce_mprfile* mprfile = ce_mprfile_open(resfile);
	return ce_resfile_close(resfile), mprfile;
}
