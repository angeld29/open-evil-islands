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
#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "cemobmanager.h"

static const char* ce_mob_dirs[] = {"Maps"};

enum {
	CE_MOB_DIR_COUNT = sizeof(ce_mob_dirs) / sizeof(ce_mob_dirs[0]),
};

struct ce_mob_manager* ce_mob_manager;

void ce_mob_manager_init(void)
{
	char path[ce_root.ei_path->length + 16];

	for (size_t i = 0; i < CE_MOB_DIR_COUNT; ++i) {
		snprintf(path, sizeof(path), "%s/%s",
			ce_root.ei_path->str, ce_mob_dirs[i]);
		ce_logging_write("mob manager: using path '%s'", path);
	}

	ce_mob_manager = ce_alloc(sizeof(struct ce_mob_manager));
}

void ce_mob_manager_term(void)
{
	if (NULL != ce_mob_manager) {
		ce_free(ce_mob_manager, sizeof(struct ce_mob_manager));
	}
}

ce_mobfile* ce_mob_manager_open(const char* name)
{
	char path[ce_root.ei_path->length + strlen(name) + 32];

	for (size_t i = 0; i < CE_MOB_DIR_COUNT; ++i) {
		snprintf(path, sizeof(path), "%s/%s/%s.mob",
			ce_root.ei_path->str, ce_mob_dirs[i], name);
		ce_mobfile* mobfile = ce_mobfile_open(path);
		if (NULL != mobfile) {
			return mobfile;
		}
	}

	return NULL;
}
