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

#ifndef CE_SCENEMNG_H
#define CE_SCENEMNG_H

#include "cevector.h"
#include "cerendqueue.h"
#include "cetimer.h"
#include "ceinput.h"
#include "cefps.h"
#include "cecamera.h"
#include "ceterrain.h"
#include "cefont.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_rendqueue* rendqueue;
	ce_timer* timer;
	ce_fps* fps;
	ce_camera* camera;
	ce_vector* terrains;
	ce_font* font;
} ce_scenemng;

extern ce_scenemng* ce_scenemng_new(void);
extern void ce_scenemng_del(ce_scenemng* rendlayer);

extern void ce_scenemng_advance(ce_scenemng* scenemng);
extern void ce_scenemng_render(ce_scenemng* scenemng);

extern ce_terrain* ce_scenemng_load_zone(ce_scenemng* scenemng,
										const char* zone_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SCENEMNG_H */
