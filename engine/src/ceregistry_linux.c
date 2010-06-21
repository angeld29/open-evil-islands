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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "cestr.h"
#include "cepath.h"
#include "celogging.h"
#include "ceconfigfile.h"
#include "ceregistry.h"

char* ce_registry_get_string_value(char* value, size_t size,
									ce_registry_key key,
									const char* key_name,
									const char* value_name)
{
	// TODO: implement it
	return NULL;

	char path[CE_PATH_MAX];

	const char* home_path = getenv("HOME");
	if (NULL == home_path) {
		ce_logging_error("registry: could not get environment variable 'HOME'");
		return NULL;
	}

	const char* reg_name = "unknown";
	switch (key) {
	case CE_REGISTRY_KEY_CURRENT_USER:
		reg_name = "user.reg";
		break;
	case CE_REGISTRY_KEY_LOCAL_MACHINE:
		reg_name = "system.reg";
		break;
	}

	if (NULL == ce_path_join(path, sizeof(path),
							home_path, ".wine", reg_name, NULL)) {
		return NULL;
	}

	ce_config_file* config_file = ce_config_file_open(path);
	if (NULL == config_file) {
		return NULL;
	}

	const char* reg_value = ce_config_file_find(config_file, key_name, value_name);
	if (NULL == reg_value) {
		value = NULL;
	} else {
		ce_strlcpy(value, reg_value, size);
		ce_logging_debug("%s", reg_value);
	}

	ce_config_file_close(config_file);
	return value;
}
