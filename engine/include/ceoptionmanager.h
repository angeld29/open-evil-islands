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

#ifndef CE_OPTIONMANAGER_H
#define CE_OPTIONMANAGER_H

#include <stdbool.h>

#include "cestring.h"
#include "ceoptparse.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_option_manager {
	ce_string* ei_path;
	ce_string* ce_path;
	int window_width;
	int window_height;
	bool fullscreen;
	int fullscreen_width;
	int fullscreen_height;
	int fullscreen_bpp;
	int fullscreen_rate;
	int fullscreen_rotation;
	bool fullscreen_reflection_x;
	bool fullscreen_reflection_y;
	bool list_video_modes;
	bool list_video_rotations;
	bool list_video_reflections;
	bool inverse_trackball;
	bool inverse_trackball_x;
	bool inverse_trackball_y;
	bool terrain_tiling;
	int terrain_lod;
	bool texture_caching;
	int thread_count;
	bool disable_sound;
	bool show_axes;
	bool show_fps;
}* ce_option_manager;

extern void ce_option_manager_init(ce_optparse* optparse);
extern void ce_option_manager_term(void);

extern ce_optparse* ce_option_manager_create_option_parser(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_OPTIONMANAGER_H */
