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

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "ceconfigfile.h"
#include "ceconfigmanager.h"

static const char* ce_config_movie_sections[CE_CONFIG_MOVIE_COUNT] = {
	[CE_CONFIG_MOVIE_START] = "Start",
	[CE_CONFIG_MOVIE_CRDTFIN] = "Crdtfin",
	[CE_CONFIG_MOVIE_CRDTFOUT] = "Crdtfout",
	[CE_CONFIG_MOVIE_TITLESFIN] = "Titlesfin",
	[CE_CONFIG_MOVIE_TITLESFOUT] = "Titlesfout",
};

struct ce_config_manager* ce_config_manager;

static void ce_config_manager_init_movies(void)
{
	for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
		ce_config_manager->movies[i] = ce_vector_new_reserved(4);
	}

	char path[ce_option_manager->ei_path->length + 32];
	snprintf(path, sizeof(path), "%s/Config/movie.ini", ce_option_manager->ei_path->str);

	ce_config_file* config_file = ce_config_file_open(path);
	if (NULL != config_file) {
		for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
			const char* line = ce_config_file_find(config_file,
								ce_config_movie_sections[i], "movies");
			if (NULL != line) { // may be NULL (commented by user)
				char buffer[strlen(line) + 1], *temp = buffer, *name;
				// lowercase it to avoid problems on case-sensitive systems
				// (seems all EI movies are in lower case)
				ce_strlwr(buffer, line);
				for (name = ce_strsep(&temp, ",");
						0 != strlen(name); name = ce_strsep(&temp, ",")) {
					ce_vector_push_back(ce_config_manager->movies[i],
										ce_string_new_str(name));
				}
			}
		}
		ce_config_file_close(config_file);
	} else {
		ce_logging_error("config manager: could not read movie configuration");
	}
}

void ce_config_manager_init(void)
{
	ce_config_manager = ce_alloc_zero(sizeof(struct ce_config_manager));
	ce_config_manager_init_movies();
}

void ce_config_manager_term(void)
{
	if (NULL != ce_config_manager) {
		for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
			ce_vector_for_each(ce_config_manager->movies[i], ce_string_del);
			ce_vector_del(ce_config_manager->movies[i]);
		}
		ce_free(ce_config_manager, sizeof(struct ce_config_manager));
	}
}
