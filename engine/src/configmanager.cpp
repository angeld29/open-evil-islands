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

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

#include "alloc.hpp"
#include "str.hpp"
#include "path.hpp"
#include "logging.hpp"
#include "optionmanager.hpp"
#include "resourcemanager.hpp"
#include "resball.hpp"
#include "regfile.hpp"
#include "configfile.hpp"
#include "configmanager.hpp"

namespace cursedearth
{
    struct ce_config_manager* ce_config_manager;

    const char* ce_config_dir = "Config";
    const char* ce_resource_dir = "Res";

    const char* ce_config_light_files[CE_CONFIG_LIGHT_COUNT] = {
        "lightsgipat.ini",
        "lightsingos.ini",
        "lightssuslanger.ini",
        "lightscavegipat.ini",
        "lightscaveingos.ini",
        "lightscavesuslanger.ini"
    };

    const char* ce_config_movie_sections[CE_CONFIG_MOVIE_COUNT] = {
        "Start",
        "Crdtfin",
        "Crdtfout",
        "Titlesfin",
        "Titlesfout"
    };

    const char* ce_config_music_sections[CE_CONFIG_MUSIC_CHAPTER_COUNT] = {
        "common",
        "gipat",
        "ingos",
        "suslanger",
        "final"
    };

    const char* ce_config_music_names[CE_CONFIG_MUSIC_ITEM_COUNT] = {
        "CalmDungeon",
        "Combat",
        "Constructor",
        "Briefing",
        "CalmOpen",
        "MainMenu",
        "Credits"
    };

    const char* ce_config_menu_type_names[CE_CONFIG_MENU_TYPE_COUNT] = {
        "MainMenu",
        "EscMenu"
    };

    const char* ce_config_menu_item_names[CE_CONFIG_MENU_TYPE_COUNT][CE_CONFIG_MENU_ITEM_COUNT] = {
        {
            "NewGame",
            "LoadGame",
            "Multiplayer",
            "Options",
            "Credits",
            "ExitGame"
        },
        {
            "ResumeGame",
            "SaveGame",
            "LoadGame",
            "Options",
            "ExitMM",
            "ExitMM"
        }
    };

    bool ce_config_manager_read_light(color_t section[24], const char* section_name, ce_config_file* config_file)
    {
        for (size_t i = 0; i < 24; ++i) {
            section[i] = CE_COLOR_WHITE;
        }

        size_t section_index = ce_config_file_section_index(config_file, section_name);
        if (section_index == ce_config_file_section_count(config_file)) {
            ce_logging_error("config manager: could not find section `%s'", section_name);
            return false;
        }

        char option_name[8], option[16], *temp;

        for (size_t i = 0; i < 24; ++i) {
            snprintf(option_name, sizeof(option_name), "time%02zu", i);

            size_t option_index = ce_config_file_option_index(config_file, section_index, option_name);
            if (option_index == ce_config_file_option_count(config_file, section_index)) {
                ce_logging_error("config manager: section `%s': could not find option `%s'", section_name, option_name);
                return false;
            }

            if (sizeof(option) <= ce_strlcpy(option, ce_config_file_get(config_file, section_index, option_index), sizeof(option))) {
                ce_logging_error("config manager: option is too long `%s'", option);
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

    void ce_config_manager_init_lights(void)
    {
        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 32);
        for (size_t i = 0; i < CE_CONFIG_LIGHT_COUNT; ++i) {
            ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_config_dir, ce_config_light_files[i], NULL);

            ce_config_file* config_file = ce_config_file_open(path.data());
            if (NULL != config_file) {
                bool sky_ok = ce_config_manager_read_light(ce_config_manager->lights[i].sky, "sky", config_file);
                bool ambient_ok = ce_config_manager_read_light(ce_config_manager->lights[i].ambient, "ambient", config_file);
                bool sunlight_ok = ce_config_manager_read_light(ce_config_manager->lights[i].sunlight, "sunlight", config_file);
                if (!sky_ok || !ambient_ok || !sunlight_ok) {
                    ce_logging_error("config manager: `%s' contains broken content", path.data());
                }
                ce_config_file_close(config_file);
            } else {
                ce_logging_error("config manager: could not read light configuration");
            }
        }
    }

    void ce_config_manager_init_movies(void)
    {
        for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
            ce_config_manager->movies[i] = ce_vector_new_reserved(4);
        }

        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 32);
        ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_config_dir, "movie.ini", NULL);

        ce_config_file* config_file = ce_config_file_open(path.data());
        if (NULL != config_file) {
            for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
                // may be NULL (commented by user)
                const char* line = ce_config_file_find(config_file, ce_config_movie_sections[i], "movies");
                if (NULL != line) {
                    std::vector<char> buffer(strlen(line) + 1);
                    char *pos = buffer.data(), *name;
                    // lowercase it to avoid problems on case-sensitive systems
                    // (seems all EI movies are in lower case)
                    ce_strlwr(buffer.data(), line);
                    do {
                        name = ce_strsep(&pos, ",");
                        if ('\0' != name[0]) {
                            ce_vector_push_back(ce_config_manager->movies[i], ce_string_new_str(name));
                        }
                    } while (NULL != pos);
                }
            }
            ce_config_file_close(config_file);
        } else {
            ce_logging_error("config manager: could not read movie configuration");
        }
    }

    void ce_config_manager_init_music(void)
    {
        for (size_t i = 0; i < CE_CONFIG_MUSIC_CHAPTER_COUNT; ++i) {
            for (size_t j = 0; j < CE_CONFIG_MUSIC_ITEM_COUNT; ++j) {
                ce_config_manager->music[i][j] = ce_vector_new_reserved(4);
            }
        }

        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 32);
        ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_resource_dir, "music.reg", NULL);

        ce_mem_file* mem_file = ce_mem_file_new_path(path.data());
        if (NULL != mem_file) {
            ce_reg_file* reg_file = ce_reg_file_new(mem_file);
            for (size_t i = 0; i < CE_CONFIG_MUSIC_CHAPTER_COUNT; ++i) {
                for (size_t j = 0; j < CE_CONFIG_MUSIC_ITEM_COUNT; ++j) {
                    for (size_t k = 0; ; ++k) {
                        ce_value* value = ce_reg_file_find(reg_file, ce_config_music_sections[i], ce_config_music_names[j], k);
                        if (NULL == value) {
                            break;
                        }
                        ce_vector_push_back(ce_config_manager->music[i][j], ce_string_new_str(ce_value_get_string(value)));
                    }
                }
            }
            ce_reg_file_del(reg_file);
            ce_mem_file_del(mem_file);
        } else {
            ce_logging_error("config manager: could not open file `%s'", path.data());
            ce_logging_error("config manager: could not read music configuration");
        }
    }

    void ce_config_manager_init_menus(void)
    {
        if (NULL != ce_resource_manager->menus) {
            ce_mem_file* mem_file = ce_res_ball_extract_mem_file_by_name(ce_resource_manager->menus, "menus.reg");
            assert(NULL != mem_file && "menus.reg required");
            ce_reg_file* reg_file = ce_reg_file_new(mem_file);
            for (size_t i = 0; i < CE_CONFIG_MENU_TYPE_COUNT; ++i) {
                for (size_t j = 0; j < CE_CONFIG_MENU_ITEM_COUNT; ++j) {
                    ce_config_manager->menu_geometry[i][j].x = ce_value_get_int(ce_reg_file_find(reg_file, ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 0));
                    ce_config_manager->menu_geometry[i][j].y = ce_value_get_int(ce_reg_file_find(reg_file, ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 1));
                    ce_config_manager->menu_geometry[i][j].width = ce_value_get_int(ce_reg_file_find(reg_file, ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 2));
                    ce_config_manager->menu_geometry[i][j].height = ce_value_get_int(ce_reg_file_find(reg_file, ce_config_menu_type_names[i], ce_config_menu_item_names[i][j], 3));
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
        ce_config_manager = (struct ce_config_manager*)ce_alloc_zero(sizeof(struct ce_config_manager));

        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 32);
        ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_config_dir, NULL);

        ce_logging_info("config manager: using path `%s'", path.data());

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
                    ce_vector_for_each(ce_config_manager->music[i][j], (void (*)(void*))ce_string_del);
                    ce_vector_del(ce_config_manager->music[i][j]);
                }
            }
            for (size_t i = 0; i < CE_CONFIG_MOVIE_COUNT; ++i) {
                ce_vector_for_each(ce_config_manager->movies[i], (void (*)(void*))ce_string_del);
                ce_vector_del(ce_config_manager->movies[i]);
            }
            ce_free(ce_config_manager, sizeof(struct ce_config_manager));
        }
    }
}
