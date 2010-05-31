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

#ifndef CE_SOUNDINSTANCE_H
#define CE_SOUNDINSTANCE_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cethread.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_SOUNDINSTANCE_STATE_STOPPED,
	CE_SOUNDINSTANCE_STATE_PAUSED,
	CE_SOUNDINSTANCE_STATE_PLAYING,
} ce_soundinstance_state;

typedef struct ce_soundinstance ce_soundinstance;

typedef struct {
	size_t size;
	bool (*ctor)(ce_soundinstance* soundinstance, va_list args);
	void (*dtor)(ce_soundinstance* soundinstance);
	size_t (*read)(ce_soundinstance* soundinstance, void* buffer, size_t size);
} ce_soundinstance_vtable;

struct ce_soundinstance {
	bool done;
	ce_soundinstance_state state;
	int bps, rate, channels;
	ce_thread_mutex* mutex;
	ce_thread_cond* cond;
	ce_thread* thread;
	ce_soundinstance_vtable vtable;
	char impl[];
};

extern ce_soundinstance* ce_soundinstance_new(ce_soundinstance_vtable vtable, ...);
extern void ce_soundinstance_del(ce_soundinstance* soundinstance);

extern void ce_soundinstance_play(ce_soundinstance* soundinstance);

extern ce_soundinstance* ce_soundinstance_create_path(const char* path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SOUNDINSTANCE_H */
