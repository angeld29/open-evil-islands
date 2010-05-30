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

#ifndef CE_SOUNDDRIVER_H
#define CE_SOUNDDRIVER_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cethread.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_sounddriver ce_sounddriver;

typedef struct {
	size_t size;
	bool (*ctor)(ce_sounddriver* sounddriver, va_list args);
	void (*dtor)(ce_sounddriver* sounddriver);
	void (*write)(ce_sounddriver* sounddriver, const void* block);
} ce_sounddriver_vtable;

struct ce_sounddriver {
	bool done;
	int bps, rate, channels;
	size_t sample_size;
	size_t block_size;
	size_t block_index;
	ce_vector* blocks;
	ce_thread_sem* free_blocks;
	ce_thread_sem* used_blocks;
	ce_thread* thread;
	ce_sounddriver_vtable vtable;
	char impl[];
};

extern ce_sounddriver* ce_sounddriver_new(ce_sounddriver_vtable vtable, ...);
extern void ce_sounddriver_del(ce_sounddriver* sounddriver);

extern void* ce_sounddriver_map_block(ce_sounddriver* sounddriver);
extern void ce_sounddriver_unmap_block(ce_sounddriver* sounddriver);

extern ce_sounddriver* ce_sounddriver_create_platform(int bps, int rate, int channels);
extern ce_sounddriver* ce_sounddriver_create_null(int bps, int rate, int channels);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SOUNDDRIVER_H */
