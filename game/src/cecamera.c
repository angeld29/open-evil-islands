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
	camera->eye = CE_VEC3_ZERO;
	camera->look = CE_QUAT_IDENTITY;
	camera->proj_changed = true;
	camera->eye_changed = true;
	camera->look_changed = true;
	return camera;
}

void ce_camera_del(ce_camera* camera)
{
	ce_free(camera, sizeof(ce_camera));
}

ce_vec3* ce_camera_get_forward(ce_camera* camera, ce_vec3* forward)
{
	ce_quat q;
	return ce_vec3_rot(forward, &CE_VEC3_NEG_UNIT_Z,
						ce_quat_conj(&q, &camera->look));
}

ce_vec3* ce_camera_get_up(ce_camera* camera, ce_vec3* up)
{
	ce_quat q;
	return ce_vec3_rot(up, &CE_VEC3_UNIT_Y,
						ce_quat_conj(&q, &camera->look));
}

ce_vec3* ce_camera_get_right(ce_camera* camera, ce_vec3* right)
{
	ce_quat q;
	return ce_vec3_rot(right, &CE_VEC3_UNIT_X,
						ce_quat_conj(&q, &camera->look));
}

void ce_camera_set_fov(ce_camera* camera, float fov)
{
	camera->fov = fov;
	camera->proj_changed = true;
}

void ce_camera_set_aspect(ce_camera* camera, int width, int height)
{
	camera->aspect = (float)width / height;
	camera->proj_changed = true;
}

void ce_camera_set_near(ce_camera* camera, float near)
{
	camera->near = near;
	camera->proj_changed = true;
}

void ce_camera_set_far(ce_camera* camera, float far)
{
	camera->far = far;
	camera->proj_changed = true;
}

void ce_camera_set_eye(ce_camera* camera, const ce_vec3* eye)
{
	ce_vec3_copy(&camera->eye, eye);
	camera->eye_changed = true;
}

void ce_camera_set_look(ce_camera* camera, const ce_quat* look)
{
	ce_quat_copy(&camera->look, look);
	camera->look_changed = true;
}

void ce_camera_move(ce_camera* camera, float offset_x, float offset_z)
{
	ce_vec3 forward, right;

	ce_camera_get_forward(camera, &forward);
	ce_camera_get_right(camera, &right);

	// ignore pitch difference angle
	forward.y = 0.0f;
	right.y = 0.0f;

	ce_vec3_normalise(&forward, &forward);
	ce_vec3_normalise(&right, &right);

	ce_vec3_scale(&forward, offset_z, &forward);
	ce_vec3_scale(&right, offset_x, &right);

	ce_vec3_add(&camera->eye, &camera->eye, &forward);
	ce_vec3_add(&camera->eye, &camera->eye, &right);

	camera->eye_changed = true;
}

void ce_camera_zoom(ce_camera* camera, float offset)
{
	ce_vec3 forward;
	ce_camera_get_forward(camera, &forward);
	ce_vec3_scale(&forward, offset, &forward);
	ce_vec3_add(&camera->eye, &camera->eye, &forward);
	camera->eye_changed = true;
}

void ce_camera_yaw_pitch(ce_camera* camera, float psi, float theta)
{
	ce_vec3 y;
	ce_quat q, t;
	ce_vec3_rot(&y, &CE_VEC3_UNIT_Y, &camera->look);
	ce_quat_mul(&t, ce_quat_init_polar(&q, psi, &y), &camera->look);
	ce_quat_mul(&camera->look, ce_quat_init_polar(&q, theta, &CE_VEC3_UNIT_X), &t);
	camera->look_changed = true;
}
