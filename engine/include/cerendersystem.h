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

#ifndef CE_RENDERSYSTEM_H
#define CE_RENDERSYSTEM_H

#include "cethread.h"
#include "cevec3.h"
#include "cequat.h"
#include "cecolor.h"
#include "ceviewport.h"
#include "cecamera.h"
#include "cetexture.h"
#include "cematerial.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_thread_id thread_id;
	char impl[];
} ce_rendersystem;

extern ce_rendersystem* ce_rendersystem_new(void);
extern void ce_rendersystem_del(ce_rendersystem* rendersystem);

extern void ce_rendersystem_begin_render(ce_rendersystem* rendersystem,
										const ce_color* clear_color);
extern void ce_rendersystem_end_render(ce_rendersystem* rendersystem);

extern void ce_rendersystem_draw_axes(ce_rendersystem* rendersystem);

// draw a cube centered at zero with side of 2
extern void ce_rendersystem_draw_wire_cube(ce_rendersystem* rendersystem);
extern void ce_rendersystem_draw_solid_cube(ce_rendersystem* rendersystem);

// draw a fullscreen texture
extern void ce_rendersystem_draw_fullscreen_texture(ce_rendersystem* rendersystem,
													ce_texture* texture);

extern void ce_rendersystem_setup_viewport(ce_rendersystem* rendersystem,
												ce_viewport* viewport);

extern void ce_rendersystem_setup_camera(ce_rendersystem* rendersystem,
												ce_camera* camera);

extern void ce_rendersystem_begin_occlusion_test(ce_rendersystem* rendersystem);
extern void ce_rendersystem_end_occlusion_test(ce_rendersystem* rendersystem);

extern void ce_rendersystem_apply_color(ce_rendersystem* rendersystem,
										const ce_color* color);

extern void
ce_rendersystem_apply_transform(ce_rendersystem* rendersystem,
									const ce_vec3* translation,
									const ce_quat* rotation,
									const ce_vec3* scaling);
extern void ce_rendersystem_discard_transform(ce_rendersystem* rendersystem);

extern void
ce_rendersystem_apply_material(ce_rendersystem* rendersystem,
									ce_material* material);
extern void
ce_rendersystem_discard_material(ce_rendersystem* rendersystem,
									ce_material* material);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERSYSTEM_H */
