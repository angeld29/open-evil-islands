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
#include "cemobmanager.h"

ce_mob_manager* ce_mob_manager_new(const char* path)
{
	ce_logging_write("mob manager: root path is '%s'", path);

	ce_mob_manager* mob_manager = ce_alloc(sizeof(ce_mob_manager));
	mob_manager->path = ce_string_new_str(path);
	return mob_manager;
}

void ce_mob_manager_del(ce_mob_manager* mob_manager)
{
	if (NULL != mob_manager) {
		ce_string_del(mob_manager->path);
		ce_free(mob_manager, sizeof(ce_mob_manager));
	}
}

ce_mobfile* ce_mob_manager_open(ce_mob_manager* mob_manager, const char* name)
{
	char path[mob_manager->path->length + strlen(name) + 5 + 1];
	snprintf(path, sizeof(path), "%s/%s.mob", mob_manager->path->str, name);
	return ce_mobfile_open(path);
}
