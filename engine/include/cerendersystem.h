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

#ifndef CE_RENDERSYSTEM_H
#define CE_RENDERSYSTEM_H

#include "cevec3.h"
#include "cequat.h"
#include "cecolor.h"
#include "cethread.h"
#include "ceviewport.h"
#include "cecamera.h"
#include "cetexture.h"
#include "cematerial.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_render_system {
	ce_thread_id thread_id;
	char impl[];
}* ce_render_system;

extern void ce_render_system_init(void);
extern void ce_render_system_term(void);

extern void ce_render_system_begin_render(const ce_color* clear_color);
extern void ce_render_system_end_render(void);

extern void ce_render_system_draw_axes(void);

// draw a cube centered at zero with side of 2
extern void ce_render_system_draw_wire_cube(void);
extern void ce_render_system_draw_solid_cube(void);

// draw a sphere centered at zero with radius of 1
extern void ce_render_system_draw_solid_sphere(void);

extern void ce_render_system_draw_fullscreen_wire_rect(unsigned int width,
														unsigned int height);

extern void ce_render_system_setup_viewport(ce_viewport* viewport);
extern void ce_render_system_setup_camera(ce_camera* camera);

extern void ce_render_system_apply_color(const ce_color* color);

extern void ce_render_system_apply_transform(const ce_vec3* translation,
											const ce_quat* rotation,
											const ce_vec3* scaling);
extern void ce_render_system_discard_transform(void);

extern void ce_render_system_apply_material(ce_material* material);
extern void ce_render_system_discard_material(ce_material* material);

#ifdef __cplusplus
}
#endif

#endif /* CE_RENDERSYSTEM_H */
