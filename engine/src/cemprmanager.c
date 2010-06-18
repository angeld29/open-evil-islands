/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include "cealloc.h"
#include "celogging.h"
#include "cepath.h"
#include "ceresfile.h"
#include "ceoptionmanager.h"
#include "cemprmanager.h"

struct ce_mpr_manager* ce_mpr_manager;

static const char* ce_mpr_dirs[] = {"Maps", NULL};
static const char* ce_mpr_exts[] = {".mpr", NULL};

void ce_mpr_manager_init(void)
{
	char path[ce_option_manager->ei_path->length + 16];
	for (size_t i = 0; NULL != ce_mpr_dirs[i]; ++i) {
		ce_path_join_clear(path, sizeof(path),
			ce_option_manager->ei_path->str, ce_mpr_dirs[i], NULL);
		ce_logging_write("mpr manager: using path '%s'", path);
	}

	ce_mpr_manager = ce_alloc_zero(sizeof(struct ce_mpr_manager));
}

void ce_mpr_manager_term(void)
{
	if (NULL != ce_mpr_manager) {
		ce_free(ce_mpr_manager, sizeof(struct ce_mpr_manager));
	}
}

ce_mprfile* ce_mpr_manager_open(const char* name)
{
	char path[ce_option_manager->ei_path->length + strlen(name) + 32];
	if (NULL == ce_path_find_special1(path, sizeof(path),
										ce_option_manager->ei_path->str,
										name, ce_mpr_dirs, ce_mpr_exts)) {
		return NULL;
	}

	ce_resfile* resfile = ce_resfile_open_file(path);
	if (NULL == resfile) {
		return NULL;
	}

	ce_mprfile* mprfile = ce_mprfile_open(resfile);
	ce_resfile_close(resfile);

	return mprfile;
}
