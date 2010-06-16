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

#ifndef CE_VIDEOMANAGER_H
#define CE_VIDEOMANAGER_H

#include "cevector.h"
#include "cestring.h"
#include "cevideoobject.h"
#include "cevideoinstance.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ce_video_object last_video_object;
	ce_vector* video_instances;
} ce_video_manager;

extern ce_video_manager* ce_video_manager_new(void);
extern void ce_video_manager_del(ce_video_manager* video_manager);

extern void ce_video_manager_advance(ce_video_manager* video_manager, float elapsed);

extern ce_video_object ce_video_manager_create(ce_video_manager* video_manager, const char* name);
extern ce_video_instance* ce_video_manager_find(ce_video_manager* video_manager, ce_video_object video_object);

#ifdef __cplusplus
}
#endif

#endif /* CE_VIDEOMANAGER_H */
