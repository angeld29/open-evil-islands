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
#include <assert.h>

#include "celogging.h"
#include "cealloc.h"
#include "ceresfile.h"
#include "cefigprotomng.h"

ce_figprotomng* ce_figprotomng_new(void)
{
	ce_figprotomng* figprotomng = ce_alloc_zero(sizeof(ce_figprotomng));
	if (NULL == figprotomng) {
		ce_logging_error("figprotomng: could not allocate memory");
		return NULL;
	}

	if (NULL == (figprotomng->resources = ce_vector_new())) {
		ce_figprotomng_del(figprotomng);
		return NULL;
	}

	return figprotomng;
}

void ce_figprotomng_del(ce_figprotomng* figprotomng)
{
	if (NULL != figprotomng) {
		if (NULL != figprotomng->resources) {
			for (int i = 0, n = ce_vector_count(figprotomng->resources); i < n; ++i) {
				ce_resfile_close(ce_vector_at(figprotomng->resources, i));
			}
			ce_vector_del(figprotomng->resources);
		}
		ce_free(figprotomng, sizeof(ce_figprotomng));
	}
}

bool
ce_figprotomng_register_resource(ce_figprotomng* figprotomng, const char* path)
{
	ce_resfile* resfile = ce_resfile_open_file(path);
	if (NULL == resfile) {
		ce_logging_error("figprotomng: could not open resource: '%s'", path);
		return false;
	}

	ce_vector_push_back(figprotomng->resources, resfile);
	ce_logging_write("figprotomng: loading '%s'... ok", ce_resfile_name(resfile));

	return true;
}

ce_figproto*
ce_figprotomng_get_figproto(ce_figprotomng* figprotomng, const char* figure_name)
{
	char file_name[strlen(figure_name) + 4 + 1];
	snprintf(file_name, sizeof(file_name), "%s.mod", figure_name);

	for (int i = 0, n = ce_vector_count(figprotomng->resources); i < n; ++i) {
		ce_resfile* resfile = ce_vector_at(figprotomng->resources, i);
		if (-1 != ce_resfile_node_index(resfile, file_name)) {
			return ce_figproto_new(figure_name, resfile);
		}
	}

	return NULL;
}
