/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_RENDERSYSTEM_HPP
#define CE_RENDERSYSTEM_HPP

#include "vec3.hpp"
#include "quat.hpp"
#include "color.hpp"
#include "thread.hpp"
#include "viewport.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "material.hpp"

namespace cursedearth
{
    struct ce_render_system* ce_render_system;

    void ce_render_system_init(void);
    void ce_render_system_term(void);

    ce_thread_id ce_render_system_thread_id(void);

    void ce_render_system_begin_render(const ce_color* clear_color);
    void ce_render_system_end_render(void);

    void ce_render_system_draw_axes(void);

    // draw a cube centered at zero with side of 2
    void ce_render_system_draw_wire_cube(void);
    void ce_render_system_draw_solid_cube(void);

    // draw a sphere centered at zero with radius of 1
    void ce_render_system_draw_solid_sphere(void);

    void ce_render_system_draw_fullscreen_wire_rect(unsigned int width, unsigned int height);

    void ce_render_system_setup_viewport(ce_viewport* viewport);
    void ce_render_system_setup_camera(ce_camera* camera);

    void ce_render_system_apply_color(const ce_color* color);

    void ce_render_system_apply_transform(const ce_vec3* translation, const ce_quat* rotation, const ce_vec3* scaling);
    void ce_render_system_discard_transform(void);

    void ce_render_system_apply_material(ce_material* material);
    void ce_render_system_discard_material(ce_material* material);
}

#endif
