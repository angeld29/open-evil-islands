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

#include "cethread.h"
#include "cesoundresource.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_SOUNDINSTANCE_STATE_STOPPED,
	CE_SOUNDINSTANCE_STATE_PAUSED,
	CE_SOUNDINSTANCE_STATE_PLAYING,
} ce_soundinstance_state;

typedef struct {
	volatile bool done;
	ce_soundinstance_state state;
	ce_soundresource* soundresource;
	ce_thread_mutex* mutex;
	ce_thread_cond* cond;
	ce_thread* thread;
} ce_soundinstance;

extern ce_soundinstance* ce_soundinstance_new(ce_soundresource* soundresource);
extern void ce_soundinstance_del(ce_soundinstance* soundinstance);

extern void ce_soundinstance_play(ce_soundinstance* soundinstance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SOUNDINSTANCE_H */
