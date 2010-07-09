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
 *  TODO: remarks
*/

#ifndef CE_SOUNDSYSTEM_H
#define CE_SOUNDSYSTEM_H

#include <stddef.h>
#include <stdbool.h>

#include "cethread.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CE_SOUND_SYSTEM_BITS_PER_SAMPLE = 16,
	CE_SOUND_SYSTEM_SAMPLES_PER_SECOND = 44100,
	CE_SOUND_SYSTEM_CHANNEL_COUNT = 2,
	CE_SOUND_SYSTEM_SAMPLE_SIZE = CE_SOUND_SYSTEM_CHANNEL_COUNT *
									(CE_SOUND_SYSTEM_BITS_PER_SAMPLE / 8),
	CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK = 1024,
	CE_SOUND_SYSTEM_BLOCK_SIZE = CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK *
								CE_SOUND_SYSTEM_SAMPLE_SIZE,
	CE_SOUND_SYSTEM_BLOCK_COUNT = 4,
};

typedef struct {
	size_t size;
	bool (*ctor)(void);
	void (*dtor)(void);
	bool (*write)(const void* block);
} ce_sound_system_vtable;

extern struct ce_sound_system {
	bool done;
	unsigned int samples_per_second; // actual value supported by implementation/hardware
	size_t next_block;
	char blocks[CE_SOUND_SYSTEM_BLOCK_COUNT][CE_SOUND_SYSTEM_BLOCK_SIZE];
	ce_semaphore* free_blocks;
	ce_semaphore* used_blocks;
	ce_thread* thread;
	ce_sound_system_vtable vtable;
	char impl[];
}* ce_sound_system;

extern ce_sound_system_vtable ce_sound_system_platform(void);
extern ce_sound_system_vtable ce_sound_system_null(void);

extern void ce_sound_system_init(void);
extern void ce_sound_system_term(void);

extern void* ce_sound_system_map_block(void);
extern void ce_sound_system_unmap_block(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDSYSTEM_H */
