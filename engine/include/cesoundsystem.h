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

#ifndef CE_SOUNDSYSTEM_H
#define CE_SOUNDSYSTEM_H

#include <stddef.h>
#include <stdbool.h>

#include "cethread.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  TODO: remarks
*/

enum {
	CE_SOUND_SYSTEM_BITS_PER_SAMPLE = 16,
	CE_SOUND_SYSTEM_SAMPLE_RATE = 44100,
	CE_SOUND_SYSTEM_CHANNEL_COUNT = 2,
	CE_SOUND_SYSTEM_SAMPLE_SIZE = CE_SOUND_SYSTEM_CHANNEL_COUNT *
									(CE_SOUND_SYSTEM_BITS_PER_SAMPLE / 8),
	CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK = 1024,
	CE_SOUND_SYSTEM_BLOCK_SIZE = CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK *
								CE_SOUND_SYSTEM_SAMPLE_SIZE,
	CE_SOUND_SYSTEM_BLOCK_COUNT = 4,
};

typedef struct ce_sound_system ce_sound_system;

typedef struct {
	size_t size;
	bool (*ctor)(ce_sound_system* sound_system);
	void (*dtor)(ce_sound_system* sound_system);
	bool (*write)(ce_sound_system* sound_system, const void* block);
} ce_sound_system_vtable;

struct ce_sound_system {
	volatile bool done;
	unsigned int sample_rate; // actual value supported by implementation/hardware
	size_t next_block;
	char blocks[CE_SOUND_SYSTEM_BLOCK_COUNT][CE_SOUND_SYSTEM_BLOCK_SIZE];
	ce_semaphore* free_blocks;
	ce_semaphore* used_blocks;
	ce_thread* thread;
	ce_sound_system_vtable vtable;
	char impl[];
};

extern ce_sound_system* ce_sound_system_new(ce_sound_system_vtable vtable);
extern ce_sound_system* ce_sound_system_new_platform(void);
extern ce_sound_system* ce_sound_system_new_null(void);
extern void ce_sound_system_del(ce_sound_system* sound_system);

extern void* ce_sound_system_map_block(ce_sound_system* sound_system);
extern void ce_sound_system_unmap_block(ce_sound_system* sound_system);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDSYSTEM_H */
