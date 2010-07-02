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

#ifndef CE_SOUNDMIXER_H
#define CE_SOUNDMIXER_H

#include <stdbool.h>

#include "cevector.h"
#include "cethread.h"
#include "ceringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: not implemented

extern struct ce_sound_mixer {
	bool done;
	ce_vector* ring_buffers;
	ce_thread* thread;
}* ce_sound_mixer;

extern void ce_sound_mixer_init(void);
extern void ce_sound_mixer_term(void);

extern void ce_sound_mixer_add(ce_ring_buffer* ring_buffer);
extern void ce_sound_mixer_remove(ce_ring_buffer* ring_buffer);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDMIXER_H */
