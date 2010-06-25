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

/*
 *  It's a... God Object anti-pattern implementation :)))
*/

#ifndef CE_ROOT_H
#define CE_ROOT_H

#include <stdbool.h>

#include "cetimer.h"
#include "ceinput.h"
#include "cerenderwindow.h"
#include "cesoundsystem.h"
#include "cescenemng.h"
#include "ceoptparse.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: use pointer
extern struct ce_root {
	bool done;
	bool show_bboxes;
	bool comprehensive_bbox_only;
	float animation_fps;
	ce_renderwindow* renderwindow;
	ce_sound_system* sound_system;
	ce_scenemng* scenemng;
	ce_timer* timer;
	ce_inputsupply* inputsupply;
	ce_inputevent* exit_event;
	ce_inputevent* switch_window_event;
	ce_inputevent* toggle_fullscreen_event;
	ce_inputevent* toggle_bbox_event;
	ce_renderwindow_listener renderwindow_listener;
} ce_root;

extern bool ce_root_init(ce_optparse* optparse, int argc, char* argv[]);
extern int ce_root_exec(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_ROOT_H */
