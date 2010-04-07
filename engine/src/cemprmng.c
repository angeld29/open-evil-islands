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
#include <string.h>

#include "celogging.h"
#include "cealloc.h"
#include "ceresfile.h"
#include "cemprmng.h"

ce_mprmng* ce_mprmng_new(const char* path)
{
	ce_mprmng* mprmng = ce_alloc(sizeof(ce_mprmng));
	mprmng->path = ce_string_new_str(path);
	ce_logging_write("mprmng: root path: '%s'", path);
	return mprmng;
}

void ce_mprmng_del(ce_mprmng* mprmng)
{
	if (NULL != mprmng) {
		ce_string_del(mprmng->path);
		ce_free(mprmng, sizeof(ce_mprmng));
	}
}

ce_mprfile* ce_mprmng_open_mprfile(ce_mprmng* mprmng, const char* name)
{
	char path[mprmng->path->length + strlen(name) + 5 + 1];
	snprintf(path, sizeof(path), "%s/%s.mpr", mprmng->path->str, name);

	ce_resfile* resfile = ce_resfile_open_file(path);
	if (NULL == resfile) {
		ce_logging_error("mprmng: failed to open mprfile: '%s'", name);
		return NULL;
	}

	ce_mprfile* mprfile = ce_mprfile_open(resfile);
	return ce_resfile_close(resfile), mprfile;
}
