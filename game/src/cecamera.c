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

#include "celogging.h"
#include "cealloc.h"
#include "cecamera.h"

ce_camera* ce_camera_new(void)
{
	ce_camera* cam = ce_alloc(sizeof(ce_camera));
	if (NULL == cam) {
		ce_logging_error("camera: could not allocate memory");
		return NULL;
	}
	cam->fov = 60.0f;
	cam->aspect = 1.0f;
	cam->near = 1.0f;
	cam->far = 500.0f;
	ce_vec3_zero(&cam->eye);
	ce_quat_identity(&cam->look);
	ce_mat4_identity(&cam->view);
	cam->proj_changed = true;
	cam->eye_changed = true;
	cam->look_changed = true;
	return cam;
}

void ce_camera_del(ce_camera* cam)
{
	ce_free(cam, sizeof(ce_camera));
}

float ce_camera_get_fov(ce_camera* cam)
{
	return cam->fov;
}

float ce_camera_get_aspect(ce_camera* cam)
{
	return cam->aspect;
}

float ce_camera_get_near(ce_camera* cam)
{
	return cam->near;
}

float ce_camera_get_far(ce_camera* cam)
{
	return cam->far;
}

ce_vec3* ce_camera_get_eye(ce_camera* cam, ce_vec3* eye)
{
	return ce_vec3_copy(eye, &cam->eye);
}

ce_vec3* ce_camera_get_forward(ce_camera* cam, ce_vec3* forward)
{
	ce_quat q;
	return ce_vec3_rot(forward, &CE_VEC3_NEG_UNIT_Z,
						ce_quat_conj(&q, &cam->look));
}

ce_vec3* ce_camera_get_up(ce_camera* cam, ce_vec3* up)
{
	ce_quat q;
	return ce_vec3_rot(up, &CE_VEC3_UNIT_Y, ce_quat_conj(&q, &cam->look));
}

ce_vec3* ce_camera_get_right(ce_camera* cam, ce_vec3* right)
{
	ce_quat q;
	return ce_vec3_rot(right, &CE_VEC3_UNIT_X, ce_quat_conj(&q, &cam->look));
}

void ce_camera_set_fov(ce_camera* cam, float fov)
{
	cam->fov = fov;
	cam->proj_changed = true;
}

void ce_camera_set_aspect(ce_camera* cam, int width, int height)
{
	cam->aspect = (float)width / height;
	cam->proj_changed = true;
}

void ce_camera_set_near(ce_camera* cam, float near)
{
	cam->near = near;
	cam->proj_changed = true;
}

void ce_camera_set_far(ce_camera* cam, float far)
{
	cam->far = far;
	cam->proj_changed = true;
}

void ce_camera_set_eye(ce_camera* cam, const ce_vec3* eye)
{
	ce_vec3_copy(&cam->eye, eye);
	cam->eye_changed = true;
}

void ce_camera_set_look(ce_camera* cam, const ce_quat* look)
{
	ce_quat_copy(&cam->look, look);
	cam->look_changed = true;
}

void ce_camera_move(ce_camera* cam, float offset_x, float offset_z)
{
	ce_vec3 forward, right;

	ce_camera_get_forward(cam, &forward);
	ce_camera_get_right(cam, &right);

	// ignore pitch difference angle
	forward.y = 0.0f;
	right.y = 0.0f;

	ce_vec3_normalise(&forward, &forward);
	ce_vec3_normalise(&right, &right);

	ce_vec3_scale(&forward, &forward, offset_z);
	ce_vec3_scale(&right, &right, offset_x);

	ce_vec3_add(&cam->eye, &cam->eye, &forward);
	ce_vec3_add(&cam->eye, &cam->eye, &right);

	cam->eye_changed = true;
}

void ce_camera_zoom(ce_camera* cam, float offset)
{
	ce_vec3 forward;
	ce_camera_get_forward(cam, &forward);
	ce_vec3_scale(&forward, &forward, offset);
	ce_vec3_add(&cam->eye, &cam->eye, &forward);
	cam->eye_changed = true;
}

void ce_camera_yaw_pitch(ce_camera* cam, float psi, float theta)
{
	ce_vec3 y;
	ce_quat q, t;
	ce_vec3_rot(&y, &CE_VEC3_UNIT_Y, &cam->look);
	ce_quat_mul(&t, ce_quat_init_polar(&q, psi, &y), &cam->look);
	ce_quat_mul(&cam->look, ce_quat_init_polar(&q, theta, &CE_VEC3_UNIT_X), &t);
	cam->look_changed = true;
}
