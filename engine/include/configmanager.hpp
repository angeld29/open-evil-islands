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

#ifndef CE_CONFIGMANAGER_HPP
#define CE_CONFIGMANAGER_HPP

#include <string>
#include <vector>

#include "color.hpp"
#include "rect.hpp"

namespace cursedearth
{
    enum {
        CE_CONFIG_LIGHT_GIPAT,
        CE_CONFIG_LIGHT_INGOS,
        CE_CONFIG_LIGHT_SUSLANGER,
        CE_CONFIG_LIGHT_CAVE_GIPAT,
        CE_CONFIG_LIGHT_CAVE_INGOS,
        CE_CONFIG_LIGHT_CAVE_SUSLANGER,
        CE_CONFIG_LIGHT_COUNT
    };

    enum {
        CE_CONFIG_MOVIE_START,
        CE_CONFIG_MOVIE_CRDTFIN,
        CE_CONFIG_MOVIE_CRDTFOUT,
        CE_CONFIG_MOVIE_TITLESFIN,
        CE_CONFIG_MOVIE_TITLESFOUT,
        CE_CONFIG_MOVIE_COUNT
    };

    enum {
        CE_CONFIG_MUSIC_CHAPTER_COMMON,
        CE_CONFIG_MUSIC_CHAPTER_GIPAT,
        CE_CONFIG_MUSIC_CHAPTER_INGOS,
        CE_CONFIG_MUSIC_CHAPTER_SUSLANGER,
        CE_CONFIG_MUSIC_CHAPTER_FINAL,
        CE_CONFIG_MUSIC_CHAPTER_COUNT
    };

    enum {
        CE_CONFIG_MUSIC_CALM_DUNGEON,
        CE_CONFIG_MUSIC_COMBAT,
        CE_CONFIG_MUSIC_CONSTRUCTOR,
        CE_CONFIG_MUSIC_BRIEFING,
        CE_CONFIG_MUSIC_CALM_OPEN,
        CE_CONFIG_MUSIC_MAIN_MENU,
        CE_CONFIG_MUSIC_CREDITS,
        CE_CONFIG_MUSIC_ITEM_COUNT
    };

    enum {
        CE_CONFIG_MENU_TYPE_COUNT = 2,
        CE_CONFIG_MENU_ITEM_COUNT = 6,
    };

    enum {
        CE_CONFIG_MENU_MAIN,
        CE_CONFIG_MENU_ESC,
    };

    enum {
        CE_CONFIG_MENU_MAIN_NEW_GAME,
        CE_CONFIG_MENU_MAIN_LOAD_GAME,
        CE_CONFIG_MENU_MAIN_MULTIPLAYER,
        CE_CONFIG_MENU_MAIN_OPTIONS,
        CE_CONFIG_MENU_MAIN_CREDITS,
        CE_CONFIG_MENU_MAIN_EXIT_GAME,
    };

    enum {
        CE_CONFIG_MENU_ESC_RESUME_GAME,
        CE_CONFIG_MENU_ESC_SAVE_GAME,
        CE_CONFIG_MENU_ESC_LOAD_GAME,
        CE_CONFIG_MENU_ESC_OPTIONS,
        CE_CONFIG_MENU_ESC_EXIT_MM,
        CE_CONFIG_MENU_ESC_PAD,
    };

    extern struct ce_config_manager
    {
        struct
        {
            ce_color sky[24];
            ce_color ambient[24];
            ce_color sunlight[24];
        } lights[CE_CONFIG_LIGHT_COUNT];
        std::vector<std::string> movies[CE_CONFIG_MOVIE_COUNT];
        std::vector<std::string> music[CE_CONFIG_MUSIC_CHAPTER_COUNT][CE_CONFIG_MUSIC_ITEM_COUNT];
        ce_rect menu_geometry[CE_CONFIG_MENU_TYPE_COUNT][CE_CONFIG_MENU_ITEM_COUNT];
    }* ce_config_manager;

    void ce_config_manager_init(void);
    void ce_config_manager_term(void);
}

#endif
