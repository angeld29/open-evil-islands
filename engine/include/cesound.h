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

#ifndef CE_SOUND_H
#define CE_SOUND_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_sound ce_sound;

extern ce_sound* ce_sound_new_file(FILE* file);
extern void ce_sound_del(ce_sound* sound);

extern void ce_sound_read(ce_sound* sound);

typedef struct ce_soundmng ce_soundmng;

extern ce_soundmng* ce_soundmng_new(void);
extern void ce_soundmng_del(ce_soundmng* soundmng);

extern void ce_soundmng_play(ce_soundmng* soundmng);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SOUND_H */
