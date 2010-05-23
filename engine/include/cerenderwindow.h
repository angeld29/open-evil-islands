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

#ifndef CE_RENDERWINDOW_H
#define CE_RENDERWINDOW_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cevector.h"
#include "ceinput.h"
#include "cedisplay.h"
#include "cecontext.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	unsigned long key;
	ce_input_button button;
} ce_renderwindow_keypair;

typedef struct {
	ce_vector* keypairs;
} ce_renderwindow_keymap;

extern ce_renderwindow_keymap* ce_renderwindow_keymap_new(void);
extern void ce_renderwindow_keymap_del(ce_renderwindow_keymap* keymap);

extern void ce_renderwindow_keymap_add(ce_renderwindow_keymap* keymap,
										unsigned long key, ce_input_button button);
extern void ce_renderwindow_keymap_add_array(ce_renderwindow_keymap* keymap,
										unsigned long keys[CE_IB_COUNT]);

extern void ce_renderwindow_keymap_sort(ce_renderwindow_keymap* keymap);

extern ce_input_button
ce_renderwindow_keymap_search(ce_renderwindow_keymap* keymap, unsigned long key);

typedef enum {
	CE_RENDERWINDOW_STATE_WINDOW,
	CE_RENDERWINDOW_STATE_FULLSCREEN,
	CE_RENDERWINDOW_STATE_COUNT
} ce_renderwindow_state;

typedef enum {
	CE_RENDERWINDOW_ACTION_NONE,
	CE_RENDERWINDOW_ACTION_MINIMIZE,
	CE_RENDERWINDOW_ACTION_RESTORED,
	CE_RENDERWINDOW_ACTION_COUNT
} ce_renderwindow_action;

typedef struct {
	int x, y;
	int width, height;
} ce_renderwindow_geometry;

typedef struct {
	int bpp, rate;
	ce_display_rotation rotation;
	ce_display_reflection reflection;
} ce_renderwindow_visual;

typedef struct {
	void (*resized)(void* listener, int width, int height);
	void (*closed)(void* listener);
	void* listener;
} ce_renderwindow_listener;

typedef struct ce_renderwindow ce_renderwindow;

typedef struct {
	bool (*ctor)(ce_renderwindow* renderwindow, va_list args);
	void (*dtor)(ce_renderwindow* renderwindow);
	void (*show)(ce_renderwindow* renderwindow);
	void (*minimize)(ce_renderwindow* renderwindow);
	struct {
		void (*prepare)(ce_renderwindow* renderwindow);
		void (*before_enter)(ce_renderwindow* renderwindow);
		void (*after_enter)(ce_renderwindow* renderwindow);
		void (*before_exit)(ce_renderwindow* renderwindow);
		void (*after_exit)(ce_renderwindow* renderwindow);
		void (*done)(ce_renderwindow* renderwindow);
	} fullscreen;
	void (*pump)(ce_renderwindow* renderwindow);
} ce_renderwindow_vtable;

struct ce_renderwindow {
	ce_renderwindow_state state;
	ce_renderwindow_action action;
	ce_renderwindow_geometry geometry[CE_RENDERWINDOW_STATE_COUNT];
	ce_renderwindow_visual visual;
	// request to switch in fullscreen mode when the window was restored
	bool restore_fullscreen;
	ce_displaymng* displaymng;
	ce_context* context;
	ce_input_context* input_context;
	ce_renderwindow_keymap* keymap;
	ce_vector* listeners;
	ce_renderwindow_vtable vtable;
	size_t size;
	char impl[];
};

extern ce_renderwindow* ce_renderwindow_new(ce_renderwindow_vtable vtable, size_t size, ...);
extern void ce_renderwindow_del(ce_renderwindow* renderwindow);

extern void ce_renderwindow_add_listener(ce_renderwindow* renderwindow,
										ce_renderwindow_listener* listener);

extern void ce_renderwindow_show(ce_renderwindow* renderwindow);
extern void ce_renderwindow_minimize(ce_renderwindow* renderwindow);

extern void ce_renderwindow_toggle_fullscreen(ce_renderwindow* renderwindow);

extern void ce_renderwindow_pump(ce_renderwindow* renderwindow);

extern void ce_renderwindow_emit_resized(ce_renderwindow* renderwindow, int width, int height);
extern void ce_renderwindow_emit_closed(ce_renderwindow* renderwindow);

extern ce_renderwindow* ce_renderwindow_create(int width, int height, const char* title);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERWINDOW_H */
