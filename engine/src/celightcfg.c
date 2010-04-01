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
#include <stdbool.h>

#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "celightcfg.h"

static bool ce_lightcfg_read_section(ce_color section[24],
									const char* section_name,
									ce_cfgfile* cfgfile)
{
	int section_index = ce_cfgfile_section_index(cfgfile, section_name);
	if (-1 == section_index) {
		ce_logging_error("lightcfg: could not find section: '%s'", section_name);
		return false;
	}

	char option_name[8], option[16], *temp;

	for (int i = 0; i < 24; ++i) {
		snprintf(option_name, sizeof(option_name), "time%02d", i);

		int option_index = ce_cfgfile_option_index(cfgfile, section_index,
															option_name);
		if (-1 == option_index) {
			ce_logging_error("lightcfg: section '%s': "
				"could not find option: '%s'", section_name, option_name);
			return false;
		}

		if (sizeof(option) <= ce_strlcpy(option, ce_cfgfile_get(cfgfile,
						section_index, option_index), sizeof(option))) {
			ce_logging_error("lightcfg: option is too long: '%s'", option);
			return false;
		}

		temp = option;

		section[i].r = atoi(ce_strsep(&temp, ",")) / 255.0f;
		section[i].g = atoi(ce_strsep(&temp, ",")) / 255.0f;
		section[i].b = atoi(ce_strsep(&temp, ",")) / 255.0f;
		section[i].a = 1.0f;
	}

	return true;
}

ce_lightcfg* ce_lightcfg_new(ce_cfgfile* cfgfile)
{
	ce_lightcfg* lightcfg = ce_alloc(sizeof(ce_lightcfg));
	bool ok = ce_lightcfg_read_section(lightcfg->sky, "sky", cfgfile) &&
		ce_lightcfg_read_section(lightcfg->ambient, "ambient", cfgfile) &&
		ce_lightcfg_read_section(lightcfg->sunlight, "sunlight", cfgfile);
	return ok ? lightcfg : (ce_lightcfg_del(lightcfg), NULL);
}

ce_lightcfg* ce_lightcfg_new_default(void)
{
	ce_lightcfg* lightcfg = ce_alloc(sizeof(ce_lightcfg));
	for (int i = 0; i < 24; ++i) {
		lightcfg->sky[i] = CE_COLOR_WHITE;
		lightcfg->ambient[i] = CE_COLOR_WHITE;
		lightcfg->sunlight[i] = CE_COLOR_WHITE;
	}
	return lightcfg;
}

void ce_lightcfg_del(ce_lightcfg* lightcfg)
{
	ce_free(lightcfg, sizeof(ce_lightcfg));
}
