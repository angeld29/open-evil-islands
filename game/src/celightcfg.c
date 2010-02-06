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

#include "cestr.h"
#include "celogging.h"
#include "celightcfg.h"

static bool read_section(float section[24][4],
							const char* section_name, ce_cfgfile* cfg)
{
	if (!ce_cfgfile_has_section(cfg, section_name)) {
		ce_logging_error("lightcfg: could not find section: '%s'", section_name);
		return false;
	}

	char option_name[8], option[16], *temp;

	for (int i = 0; i < 24; ++i) {
		snprintf(option_name, sizeof(option_name), "time%02d", i);

		if (!ce_cfgfile_has_option(cfg, section_name, option_name)) {
			ce_logging_error("lightcfg: section '%s': "
				"could not find option: '%s'", section_name, option_name);
			return false;
		}

		if (sizeof(option) <= ce_strlcpy(option, ce_cfgfile_get(cfg,
						section_name, option_name), sizeof(option))) {
			ce_logging_error("lightcfg: option is too long: '%s'", option);
			return false;
		}

		temp = option;
		for (int j = 0; j < 3; ++j) {
			section[i][j] = atoi(ce_strsep(&temp, ",")) / 255.0f;
		}
		section[i][3] = 1.0f;
	}

	return true;
}

bool ce_lightcfg_init(ce_lightcfg* light, ce_cfgfile* cfg)
{
	return read_section(light->sunlight, "sunlight", cfg) &&
			read_section(light->ambient, "ambient", cfg) &&
			read_section(light->sky, "sky", cfg);
}
