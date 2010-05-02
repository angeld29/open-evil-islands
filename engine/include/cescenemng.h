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

#include <stdbool.h>

#include "cethread.h"
#include "cetimer.h"
#include "ceinput.h"
#include "cetexmng.h"
#include "cemprmng.h"
#include "cefigmng.h"
#include "cemobfile.h"
#include "ceterrain.h"
#include "cefps.h"
#include "cefont.h"
#include "ceviewport.h"
#include "cecamera.h"
#include "cescenenode.h"
#include "cerenderqueue.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	bool show_axes;
	bool show_bboxes;
	bool comprehensive_bbox_only;
	bool terrain_tiling;
	int thread_count;
	float anmfps;
	bool scenenode_force_update;
	ce_scenenode* scenenode;
	ce_texmng* texmng;
	ce_mprmng* mprmng;
	ce_terrain* terrain;
	ce_figmng* figmng;
	ce_rendersystem* rendersystem;
	ce_renderqueue* renderqueue;
	ce_viewport* viewport;
	ce_camera* camera;
	ce_timer* timer;
	ce_fps* fps;
	ce_font* font;
} ce_scenemng;

extern ce_scenemng* ce_scenemng_new(const char* ei_path);
extern void ce_scenemng_del(ce_scenemng* rendlayer);

extern void ce_scenemng_advance(ce_scenemng* scenemng);
extern void ce_scenemng_render(ce_scenemng* scenemng);

extern ce_terrain*
ce_scenemng_create_terrain(ce_scenemng* scenemng,
							const char* name,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_scenenode* scenenode);

extern ce_figentity*
ce_scenemng_create_figentity(ce_scenemng* scenemng,
							const char* name,
							const ce_complection* complection,
							const ce_vec3* position,
							const ce_quat* orientation,
							int texture_count,
							const char* texture_names[],
							ce_scenenode* scenenode);

extern ce_figentity*
ce_scenemng_create_figentity_mobobject(ce_scenemng* scenemng,
									const ce_mobobject_object* mobobject);

extern void ce_scenemng_remove_figentity(ce_scenemng* scenemng,
										ce_figentity* figentity);

extern void ce_scenemng_load_mobfile(ce_scenemng* scenemng,
									const ce_mobfile* mobfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SCENEMNG_H */
