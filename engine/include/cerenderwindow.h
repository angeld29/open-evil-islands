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

#include "cevector.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_RENDERWINDOW_ROTATION_0,
	CE_RENDERWINDOW_ROTATION_90,
	CE_RENDERWINDOW_ROTATION_180,
	CE_RENDERWINDOW_ROTATION_270,
	CE_RENDERWINDOW_ROTATION_COUNT
} ce_renderwindow_rotation;

typedef enum {
	CE_RENDERWINDOW_REFLECTION_NONE,
	CE_RENDERWINDOW_REFLECTION_X,
	CE_RENDERWINDOW_REFLECTION_Y,
	CE_RENDERWINDOW_REFLECTION_COUNT
} ce_renderwindow_reflection;

typedef struct {
	int width, height, bpp, rate;
} ce_renderwindow_mode;

extern ce_renderwindow_mode*
ce_renderwindow_mode_new(int width, int height, int bpp, int rate);
extern void ce_renderwindow_mode_del(ce_renderwindow_mode* mode);

typedef struct ce_renderwindow_modemng ce_renderwindow_modemng;

typedef struct {
	void (*ctor)(ce_renderwindow_modemng* modemng, va_list args);
	void (*dtor)(ce_renderwindow_modemng* modemng);
	void (*change)(ce_renderwindow_modemng* modemng, int index);
} ce_renderwindow_modemng_vtable;

struct ce_renderwindow_modemng {
	ce_vector* modes;
	ce_renderwindow_modemng_vtable vtable;
	size_t size;
	char impl[];
};

extern ce_renderwindow_modemng*
ce_renderwindow_modemng_new(ce_renderwindow_modemng_vtable vtable, size_t size, ...);
extern void ce_renderwindow_modemng_del(ce_renderwindow_modemng* modemng);

extern void ce_renderwindow_modemng_change(ce_renderwindow_modemng* modemng, int index);

typedef struct ce_renderwindow ce_renderwindow;

extern ce_renderwindow* ce_renderwindow_new(void);
extern void ce_renderwindow_del(ce_renderwindow* renderwindow);

extern bool ce_renderwindow_pump(ce_renderwindow* renderwindow);
extern void ce_renderwindow_swap(ce_renderwindow* renderwindow);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERWINDOW_H */
