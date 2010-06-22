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
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "cepath.h"
#include "ceoptionmanager.h"
#include "ceconfigfile.h"
#include "ceconfigmanager.h"

struct ce_config_manager* ce_config_manager;

static const char* ce_config_dir = "Config";

static const char* ce_config_light_files[CE_CONFIG_LIGHT_COUNT] = {
	[CE_CONFIG_LIGHT_GIPAT] = "lightsgipat.ini",
	[CE_CONFIG_LIGHT_INGOS] = "lightsingos.ini",
	[CE_CONFIG_LIGHT_SUSLANGER] = "lightssuslanger.ini",
	[CE_CONFIG_LIGHT_CAVE_GIPAT] = "lightscavegipat.ini",
	[CE_CONFIG_LIGHT_CAVE_INGOS] = "lightscaveingos.ini",
	[CE_CONFIG_LIGHT_CAVE_SUSLANGER] = "lightscavesuslanger.ini",
};

static const char* ce_config_movie_sections[CE_CONFIG_MOVIE_COUNT] = {
	[CE_CONFIG_MOVIE_START] = "Start",
	[CE_CONFIG_MOVIE_CRDTFIN] = "Crdtfin",
	[CE_CONFIG_MOVIE_CRDTFOUT] = "Crdtfout",
	[CE_CONFIG_MOVIE_TITLESFIN] = "Titlesfin",
	[CE_CONFIG_MOVIE_TITLESFOUT] = "Titlesfout",
};

static bool ce_config_manager_read_light(ce_color section[24],
											const char* section_name,
											ce_config_file* config_file)
{
	for (size_t i = 0; i < 24; ++i) {
		section[i] = CE_COLOR_WHITE;
	}

	size_t section_index = ce_config_file_section_index(config_file, section_name);
	if (section_index == ce_config_file_section_count(config_file)) {
		ce_logging_error("config manager: could not find section '%s'", section_name);
		return false;
	}

	char option_name[8], option[16], *temp;

	for (size_t i = 0; i < 24; ++i) {
		snprintf(option_name, sizeof(option_name), "time%02zu", i);

		size_t option_index = ce_config_file_option_index(config_file, section_index, option_name);
		if (option_index == ce_config_file_option_count(config_file, section_index)) {
			ce_logging_error("config manager: section '%s': "
				"could not find option '%s'", section_name, option_name);
			return false;
		}

		if (sizeof(option) <= ce_strlcpy(option, ce_config_file_get(config_file,
						section_index, option_index), sizeof(option))) {
			ce_logging_error("config manager: option is too long '%s'", option);
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

static void ce_config_manager_init_lights(void)
{
	char path[ce_config_manager->config_path->length + 32];
	for (size_t i = 0; i < CE_CONFIG_LIGHT_COUNT; ++i) {
		ce_path_join(path, sizeof(path), ce_config_manager->
			config_path->str, ce_config_light_files[i], NULL);

		ce_config_file* config_file = ce_config_file_open(path);
		if (NULL != config_file) {
			bool sky_ok = ce_config_manager_read_light(ce_config_manager->
								lights[i].sky, "sky", config_file);
			bool ambient_ok = ce_config_manager_read_light(ce_config_manager->
								lights[i].ambient, "ambient", config_file);
			bool sunlight_ok = ce_config_manager_read_light(ce_config_manager->
									lights[i].sunlight, "sunlight", config_file);
			if (!sky_ok || !ambient_ok || !sunlight_ok) {
				ce_logging_error("config manager: '%s' contains broken content", path);
			}
			ce_config_file_close(config_file);
		} else {
			ce_logging_error("config manager: could not read light configuration");
		}
	}
}

static void ce_config_manager_init_movies(void)
{
	for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
		ce_config_manager->movies[i] = ce_vector_new_reserved(4);
	}

	char path[ce_config_manager->config_path->length + 32];
	ce_path_join(path, sizeof(path), ce_config_manager->
		config_path->str, "movie.ini", NULL);

	ce_config_file* config_file = ce_config_file_open(path);
	if (NULL != config_file) {
		for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
			const char* line = ce_config_file_find(config_file,
								ce_config_movie_sections[i], "movies");
			if (NULL != line) { // may be NULL (commented by user)
				char buffer[strlen(line) + 1], *pos = buffer, *name;
				// lowercase it to avoid problems on case-sensitive systems
				// (seems all EI movies are in lower case)
				ce_strlwr(buffer, line);
				do {
					name = ce_strsep(&pos, ",");
					if ('\0' != name[0]) {
						ce_vector_push_back(ce_config_manager->movies[i],
											ce_string_new_str(name));
					}
				} while (NULL != pos);
			}
		}
		ce_config_file_close(config_file);
	} else {
		ce_logging_error("config manager: could not read movie configuration");
	}
}

void ce_config_manager_init(void)
{
	char path[ce_option_manager->ei_path->length + 16];
	ce_path_join(path, sizeof(path),
		ce_option_manager->ei_path->str, ce_config_dir, NULL);

	ce_logging_write("config manager: using path '%s'", path);

	ce_config_manager = ce_alloc_zero(sizeof(struct ce_config_manager));
	ce_config_manager->config_path = ce_string_new_str(path);
	ce_config_manager_init_lights();
	ce_config_manager_init_movies();
}

void ce_config_manager_term(void)
{
	if (NULL != ce_config_manager) {
		for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
			ce_vector_for_each(ce_config_manager->movies[i], ce_string_del);
			ce_vector_del(ce_config_manager->movies[i]);
		}
		ce_string_del(ce_config_manager->config_path);
		ce_free(ce_config_manager, sizeof(struct ce_config_manager));
	}
}
