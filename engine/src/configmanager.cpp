/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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
#include "cepath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "ceresourcemanager.h"
#include "ceresball.h"
#include "ceregfile.h"
#include "ceconfigfile.h"
#include "ceconfigmanager.h"

struct ce_config_manager* ce_config_manager;

static const char* ce_config_dir = "Config";
static const char* ce_resource_dir = "Res";

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

static const char* ce_config_music_sections[CE_CONFIG_MUSIC_CHAPTER_COUNT] = {
    [CE_CONFIG_MUSIC_CHAPTER_COMMON] = "common",
    [CE_CONFIG_MUSIC_CHAPTER_GIPAT] = "gipat",
    [CE_CONFIG_MUSIC_CHAPTER_INGOS] = "ingos",
    [CE_CONFIG_MUSIC_CHAPTER_SUSLANGER] = "suslanger",
    [CE_CONFIG_MUSIC_CHAPTER_FINAL] = "final",
};

static const char* ce_config_music_names[CE_CONFIG_MUSIC_ITEM_COUNT] = {
    [CE_CONFIG_MUSIC_CALM_DUNGEON] = "CalmDungeon",
    [CE_CONFIG_MUSIC_COMBAT] = "Combat",
    [CE_CONFIG_MUSIC_CONSTRUCTOR] = "Constructor",
    [CE_CONFIG_MUSIC_BRIEFING] = "Briefing",
    [CE_CONFIG_MUSIC_CALM_OPEN] = "CalmOpen",
    [CE_CONFIG_MUSIC_MAIN_MENU] = "MainMenu",
    [CE_CONFIG_MUSIC_CREDITS] = "Credits",
};

static const char* ce_config_menu_type_names[CE_CONFIG_MENU_TYPE_COUNT] = {
    [CE_CONFIG_MENU_MAIN] = "MainMenu",
    [CE_CONFIG_MENU_ESC] = "EscMenu",
};

static const char* ce_config_menu_item_names[CE_CONFIG_MENU_TYPE_COUNT]
                                            [CE_CONFIG_MENU_ITEM_COUNT] = {
    [CE_CONFIG_MENU_MAIN] = {
        [CE_CONFIG_MENU_MAIN_NEW_GAME] = "NewGame",
        [CE_CONFIG_MENU_MAIN_LOAD_GAME] = "LoadGame",
        [CE_CONFIG_MENU_MAIN_MULTIPLAYER] = "Multiplayer",
        [CE_CONFIG_MENU_MAIN_OPTIONS] = "Options",
        [CE_CONFIG_MENU_MAIN_CREDITS] = "Credits",
        [CE_CONFIG_MENU_MAIN_EXIT_GAME] = "ExitGame",
    },
    [CE_CONFIG_MENU_ESC] = {
        [CE_CONFIG_MENU_ESC_RESUME_GAME] = "ResumeGame",
        [CE_CONFIG_MENU_ESC_SAVE_GAME] = "SaveGame",
        [CE_CONFIG_MENU_ESC_LOAD_GAME] = "LoadGame",
        [CE_CONFIG_MENU_ESC_OPTIONS] = "Options",
        [CE_CONFIG_MENU_ESC_EXIT_MM] = "ExitMM",
        [CE_CONFIG_MENU_ESC_PAD] = "ExitMM",
    },
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
    char path[ce_option_manager->ei_path->length + 32];
    for (size_t i = 0; i < CE_CONFIG_LIGHT_COUNT; ++i) {
        ce_path_join(path, sizeof(path), ce_option_manager->ei_path->str,
            ce_config_dir, ce_config_light_files[i], NULL);

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

    char path[ce_option_manager->ei_path->length + 32];
    ce_path_join(path, sizeof(path), ce_option_manager->ei_path->str,
        ce_config_dir, "movie.ini", NULL);

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

static void ce_config_manager_init_music(void)
{
    for (size_t i = 0; i < CE_CONFIG_MUSIC_CHAPTER_COUNT; ++i) {
        for (size_t j = 0; j < CE_CONFIG_MUSIC_ITEM_COUNT; ++j) {
            ce_config_manager->music[i][j] = ce_vector_new_reserved(4);
        }
    }

    char path[ce_option_manager->ei_path->length + 32];
    ce_path_join(path, sizeof(path), ce_option_manager->ei_path->str,
        ce_resource_dir, "music.reg", NULL);

    ce_mem_file* mem_file = ce_mem_file_new_path(path);
    if (NULL != mem_file) {
        ce_reg_file* reg_file = ce_reg_file_new(mem_file);
        for (size_t i = 0; i < CE_CONFIG_MUSIC_CHAPTER_COUNT; ++i) {
            for (size_t j = 0; j < CE_CONFIG_MUSIC_ITEM_COUNT; ++j) {
                for (size_t k = 0; ; ++k) {
                    ce_value* value = ce_reg_file_find(reg_file,
                        ce_config_music_sections[i], ce_config_music_names[j], k);
                    if (NULL == value) {
                        break;
                    }
                    ce_vector_push_back(ce_config_manager->music[i][j],
                        ce_string_new_str(ce_value_get_string(value)));
                }
            }
        }
        ce_reg_file_del(reg_file);
        ce_mem_file_del(mem_file);
    } else {
        ce_logging_error("config manager: could not open file '%s'", path);
        ce_logging_error("config manager: could not read music configuration");
    }
}

static void ce_config_manager_init_menus(void)
{
    if (NULL != ce_resource_manager->menus) {
        ce_mem_file* mem_file = ce_res_ball_extract_mem_file_by_name(ce_resource_manager->menus, "menus.reg");
        assert(NULL != mem_file && "menus.reg required");
        ce_reg_file* reg_file = ce_reg_file_new(mem_file);
        for (size_t i = 0; i < CE_CONFIG_MENU_TYPE_COUNT; ++i) {
            for (size_t j = 0; j < CE_CONFIG_MENU_ITEM_COUNT; ++j) {
                ce_config_manager->menu_geometry[i][j].x =
                    ce_value_get_int(ce_reg_file_find(reg_file,
                        ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 0));
                ce_config_manager->menu_geometry[i][j].y =
                    ce_value_get_int(ce_reg_file_find(reg_file,
                        ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 1));
                ce_config_manager->menu_geometry[i][j].width =
                    ce_value_get_int(ce_reg_file_find(reg_file,
                        ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 2));
                ce_config_manager->menu_geometry[i][j].height =
                    ce_value_get_int(ce_reg_file_find(reg_file,
                        ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 3));
                ce_config_manager->menu_geometry[i][j].width -= ce_config_manager->menu_geometry[i][j].x;
                ce_config_manager->menu_geometry[i][j].height -= ce_config_manager->menu_geometry[i][j].y;
            }
        }
        ce_reg_file_del(reg_file);
        ce_mem_file_del(mem_file);
    } else {
        ce_logging_error("config manager: could not read menus configuration");
    }
}

void ce_config_manager_init(void)
{
    ce_config_manager = ce_alloc_zero(sizeof(struct ce_config_manager));

    char path[ce_option_manager->ei_path->length + 32];
    ce_path_join(path, sizeof(path),
        ce_option_manager->ei_path->str, ce_config_dir, NULL);

    ce_logging_write("config manager: using path '%s'", path);

    ce_config_manager_init_lights();
    ce_config_manager_init_movies();
    ce_config_manager_init_music();
    ce_config_manager_init_menus();
}

void ce_config_manager_term(void)
{
    if (NULL != ce_config_manager) {
        for (size_t i = 0; i < CE_CONFIG_MUSIC_CHAPTER_COUNT; ++i) {
            for (size_t j = 0; j < CE_CONFIG_MUSIC_ITEM_COUNT; ++j) {
                ce_vector_for_each(ce_config_manager->music[i][j], ce_string_del);
                ce_vector_del(ce_config_manager->music[i][j]);
            }
        }
        for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
            ce_vector_for_each(ce_config_manager->movies[i], ce_string_del);
            ce_vector_del(ce_config_manager->movies[i]);
        }
        ce_free(ce_config_manager, sizeof(struct ce_config_manager));
    }
}
