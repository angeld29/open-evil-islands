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

extern ce_renderwindow_keymap* ce_renderwindow_keymap_new(int capacity);
extern void ce_renderwindow_keymap_del(ce_renderwindow_keymap* keymap);

extern void ce_renderwindow_keymap_add(ce_renderwindow_keymap* keymap,
										unsigned long key, ce_input_button button);

extern void ce_renderwindow_keymap_sort(ce_renderwindow_keymap* keymap);

extern ce_input_button
ce_renderwindow_keymap_search(ce_renderwindow_keymap* keymap, unsigned long key);

typedef struct {
	void (*closed)(void* listener);
	void* listener;
} ce_renderwindow_listener;

typedef struct {
	int width, height, bpp, rate;
	ce_display_rotation rotation;
	ce_display_reflection reflection;
	bool fullscreen;
	ce_displaymng* displaymng;
	ce_context* context;
	ce_input_context* input_context;
	ce_renderwindow_keymap* keymap;
	ce_vector* listeners;
	char impl[];
} ce_renderwindow;

extern ce_renderwindow* ce_renderwindow_new(const char* title, int width, int height);
extern void ce_renderwindow_del(ce_renderwindow* renderwindow);

extern void ce_renderwindow_add_listener(ce_renderwindow* renderwindow,
										ce_renderwindow_listener* listener);

extern void ce_renderwindow_show(ce_renderwindow* renderwindow);

extern void ce_renderwindow_toggle_fullscreen(ce_renderwindow* renderwindow);
extern void ce_renderwindow_minimize(ce_renderwindow* renderwindow);

extern void ce_renderwindow_pump(ce_renderwindow* renderwindow);

extern void ce_renderwindow_emit_closed(ce_renderwindow* renderwindow);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERWINDOW_H */
