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

#ifndef CE_SOUNDSYSTEM_H
#define CE_SOUNDSYSTEM_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cethread.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_soundsystem ce_soundsystem;

typedef struct {
	size_t size;
	bool (*ctor)(ce_soundsystem* soundsystem, va_list args);
	void (*dtor)(ce_soundsystem* soundsystem);
	void (*write)(ce_soundsystem* soundsystem, const void* block);
} ce_soundsystem_vtable;

/*
 *  TODO: remarks
*/

struct ce_soundsystem {
	volatile bool done;
	unsigned int bps, rate, channels;
	size_t sample_size, sample_count;
	size_t block_size, block_index;
	ce_vector* blocks;
	ce_thread_sem* free_blocks;
	ce_thread_sem* used_blocks;
	ce_thread* thread;
	ce_soundsystem_vtable vtable;
	char impl[];
};

extern ce_soundsystem* ce_soundsystem_new(ce_soundsystem_vtable vtable, ...);
extern void ce_soundsystem_del(ce_soundsystem* soundsystem);

extern void* ce_soundsystem_map_block(ce_soundsystem* soundsystem);
extern void ce_soundsystem_unmap_block(ce_soundsystem* soundsystem);

extern ce_soundsystem* ce_soundsystem_create_platform(void);
extern ce_soundsystem* ce_soundsystem_create_null(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SOUNDSYSTEM_H */
