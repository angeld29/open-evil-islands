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

#include "alloc.hpp"
#include "camera.hpp"

namespace cursedearth
{
    ce_camera* ce_camera_new(void)
    {
        ce_camera* camera = (ce_camera*)ce_alloc(sizeof(ce_camera));
        camera->fov = 60.0f;
        camera->aspect = 1.0f;
        camera->near = 1.0f;
        camera->far = 500.0f;
        camera->position = vec3_t::zero();
        camera->orientation = CE_QUAT_IDENTITY;
        return camera;
    }

    void ce_camera_del(ce_camera* camera)
    {
        ce_free(camera, sizeof(ce_camera));
    }

    vec3_t* ce_camera_get_forward(ce_camera* camera, vec3_t* forward)
    {
        ce_quat tmp;
        ce_quat_conj(&tmp, &camera->orientation);
        *forward = vec3_t::neg_unit_z();
        forward->rotate(tmp);
        return forward;
    }

    vec3_t* ce_camera_get_up(ce_camera* camera, vec3_t* up)
    {
        ce_quat tmp;
        ce_quat_conj(&tmp, &camera->orientation);
        *up = vec3_t::unit_y();
        up->rotate(tmp);
        return up;
    }

    vec3_t* ce_camera_get_right(ce_camera* camera, vec3_t* right)
    {
        ce_quat tmp;
        ce_quat_conj(&tmp, &camera->orientation);
        *right = vec3_t::unit_x();
        right->rotate(tmp);
        return right;
    }

    void ce_camera_set_fov(ce_camera* camera, float fov)
    {
        camera->fov = fov;
    }

    void ce_camera_set_aspect(ce_camera* camera, float aspect)
    {
        camera->aspect = aspect;
    }

    void ce_camera_set_near(ce_camera* camera, float near)
    {
        camera->near = near;
    }

    void ce_camera_set_far(ce_camera* camera, float far)
    {
        camera->far = far;
    }

    void ce_camera_set_position(ce_camera* camera, const vec3_t* position)
    {
        camera->position = *position;
    }

    void ce_camera_set_orientation(ce_camera* camera, const ce_quat* orientation)
    {
        camera->orientation = *orientation;
    }

    void ce_camera_move(ce_camera* camera, float xoffset, float zoffset)
    {
        vec3_t forward, right;
        ce_camera_get_forward(camera, &forward);
        ce_camera_get_right(camera, &right);

        // ignore pitch difference angle
        forward.y = 0.0f;
        right.y = 0.0f;

        forward.normalize();
        right.normalize();

        forward *= zoffset;
        right *= xoffset;

        camera->position += forward;
        camera->position += right;
    }

    void ce_camera_zoom(ce_camera* camera, float offset)
    {
        vec3_t forward;
        ce_camera_get_forward(camera, &forward);
        forward *= offset;
        camera->position += forward;
    }

    void ce_camera_yaw_pitch(ce_camera* camera, float psi, float theta)
    {
        vec3_t y = vec3_t::unit_y(), x = vec3_t::unit_x();
        ce_quat tmp, tmp2;
        y.rotate(camera->orientation);
        ce_quat_mul(&tmp2, ce_quat_init_polar(&tmp, psi, &y), &camera->orientation);
        ce_quat_mul(&camera->orientation, ce_quat_init_polar(&tmp, theta, &x), &tmp2);
    }
}
