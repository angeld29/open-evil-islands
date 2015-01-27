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

#include "cealloc.h"
#include "cecamera.h"

ce_camera* ce_camera_new(void)
{
    ce_camera* camera = ce_alloc(sizeof(ce_camera));
    camera->fov = 60.0f;
    camera->aspect = 1.0f;
    camera->near = 1.0f;
    camera->far = 500.0f;
    camera->position = CE_VEC3_ZERO;
    camera->orientation = CE_QUAT_IDENTITY;
    return camera;
}

void ce_camera_del(ce_camera* camera)
{
    ce_free(camera, sizeof(ce_camera));
}

ce_vec3* ce_camera_get_forward(ce_camera* camera, ce_vec3* forward)
{
    ce_quat tmp;
    return ce_vec3_rot(forward, &CE_VEC3_NEG_UNIT_Z,
                        ce_quat_conj(&tmp, &camera->orientation));
}

ce_vec3* ce_camera_get_up(ce_camera* camera, ce_vec3* up)
{
    ce_quat tmp;
    return ce_vec3_rot(up, &CE_VEC3_UNIT_Y,
                        ce_quat_conj(&tmp, &camera->orientation));
}

ce_vec3* ce_camera_get_right(ce_camera* camera, ce_vec3* right)
{
    ce_quat tmp;
    return ce_vec3_rot(right, &CE_VEC3_UNIT_X,
                        ce_quat_conj(&tmp, &camera->orientation));
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

void ce_camera_set_position(ce_camera* camera, const ce_vec3* position)
{
    camera->position = *position;
}

void ce_camera_set_orientation(ce_camera* camera, const ce_quat* orientation)
{
    camera->orientation = *orientation;
}

void ce_camera_move(ce_camera* camera, float xoffset, float zoffset)
{
    ce_vec3 forward, right;
    ce_camera_get_forward(camera, &forward);
    ce_camera_get_right(camera, &right);

    // ignore pitch difference angle
    forward.y = 0.0f;
    right.y = 0.0f;

    ce_vec3_norm(&forward, &forward);
    ce_vec3_norm(&right, &right);

    ce_vec3_scale(&forward, zoffset, &forward);
    ce_vec3_scale(&right, xoffset, &right);

    ce_vec3_add(&camera->position, &camera->position, &forward);
    ce_vec3_add(&camera->position, &camera->position, &right);
}

void ce_camera_zoom(ce_camera* camera, float offset)
{
    ce_vec3 forward;
    ce_camera_get_forward(camera, &forward);

    ce_vec3_scale(&forward, offset, &forward);

    ce_vec3_add(&camera->position, &camera->position, &forward);
}

void ce_camera_yaw_pitch(ce_camera* camera, float psi, float theta)
{
    ce_vec3 y;
    ce_quat tmp, tmp2;
    ce_vec3_rot(&y, &CE_VEC3_UNIT_Y, &camera->orientation);
    ce_quat_mul(&tmp2,
                ce_quat_init_polar(&tmp, psi, &y), &camera->orientation);
    ce_quat_mul(&camera->orientation,
                ce_quat_init_polar(&tmp, theta, &CE_VEC3_UNIT_X), &tmp2);
}
