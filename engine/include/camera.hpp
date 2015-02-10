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

#ifndef CE_CAMERA_HPP
#define CE_CAMERA_HPP

#include "vector3.hpp"
#include "quaternion.hpp"

namespace cursedearth
{
    typedef struct {
        float fov;
        float aspect;
        float m_near;
        float m_far;
        vector3_t position;
        quaternion_t orientation;
    } ce_camera;

    ce_camera* ce_camera_new(void);
    void ce_camera_del(ce_camera* camera);

    vector3_t* ce_camera_get_forward(ce_camera* camera, vector3_t* forward);
    vector3_t* ce_camera_get_up(ce_camera* camera, vector3_t* up);
    vector3_t* ce_camera_get_right(ce_camera* camera, vector3_t* right);

    void ce_camera_set_fov(ce_camera* camera, float fov);
    void ce_camera_set_aspect(ce_camera* camera, float aspect);
    void ce_camera_set_near(ce_camera* camera, float near);
    void ce_camera_set_far(ce_camera* camera, float far);

    void ce_camera_set_position(ce_camera* camera, const vector3_t* position);
    void ce_camera_set_orientation(ce_camera* camera, const quaternion_t* orientation);

    void ce_camera_move(ce_camera* camera, float xoffset, float zoffset);
    void ce_camera_zoom(ce_camera* camera, float offset);
    void ce_camera_yaw_pitch(ce_camera* camera, float psi, float theta);
}

#endif
