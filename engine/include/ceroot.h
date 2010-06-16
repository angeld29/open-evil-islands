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

#ifndef CE_ROOT_H
#define CE_ROOT_H

#include <stdbool.h>

#include "cestring.h"
#include "cethread.h"
#include "cetimer.h"
#include "ceinput.h"
#include "cerenderwindow.h"
#include "cerendersystem.h"
#include "cesoundsystem.h"
#include "cesoundmanager.h"
#include "cevideomanager.h"
#include "cetexmng.h"
#include "cemprmng.h"
#include "cemobmanager.h"
#include "cefigmng.h"
#include "cescenemng.h"
#include "ceoptparse.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_root {
	bool inited;
	bool done;
	bool terrain_tiling;
	bool inverse_trackball_x;
	bool inverse_trackball_y;
	bool show_axes;
	bool show_fps;
	bool show_bboxes;
	bool comprehensive_bbox_only;
	int thread_count;
	float anmfps;
	ce_string* ei_path;
	ce_string* ce_path;
	ce_timer* timer;
	ce_threadpool* threadpool;
	ce_renderwindow* renderwindow;
	ce_rendersystem* rendersystem;
	ce_sound_system* sound_system;
	ce_sound_manager* sound_manager;
	ce_video_manager* video_manager;
	ce_texmng* texmng;
	ce_mprmng* mprmng;
	ce_mob_manager* mob_manager;
	ce_figmng* figmng;
	ce_scenemng* scenemng;
	ce_inputsupply* inputsupply;
	ce_inputevent* exit_event;
	ce_inputevent* switch_window_event;
	ce_inputevent* toggle_fullscreen_event;
	ce_inputevent* toggle_bbox_event;
	ce_renderwindow_listener renderwindow_listener;
} ce_root;

extern bool ce_root_init(ce_optparse* optparse);

extern int ce_root_exec(void);

extern ce_optparse* ce_root_create_optparse(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_ROOT_H */
