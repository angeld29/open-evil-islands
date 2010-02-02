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

#include <stdbool.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "cealloc.h"
#include "vec3.h"
#include "quat.h"
#include "mat4.h"
#include "cecamera.h"

struct ce_camera {
	float fov;
	float aspect;
	float near;
	float far;
	vec3 eye;
	quat look;
	mat4 view;
	bool proj_changed;
	bool eye_changed;
	bool look_changed;
};

static void update_rotation(mat4* view, const quat* look)
{
	float tx  = 2.0f * look->x;
	float ty  = 2.0f * look->y;
	float tz  = 2.0f * look->z;
	float twx = tx * look->w;
	float twy = ty * look->w;
	float twz = tz * look->w;
	float txx = tx * look->x;
	float txy = ty * look->x;
	float txz = tz * look->x;
	float tyy = ty * look->y;
	float tyz = tz * look->y;
	float tzz = tz * look->z;

	view->m[0] = 1.0f - (tyy + tzz);
	view->m[1] = txy + twz;
	view->m[2] = txz - twy;

	view->m[4] = txy - twz;
	view->m[5] = 1.0f - (txx + tzz);
	view->m[6] = tyz + twx;

	view->m[8] = txz + twy;
	view->m[9] = tyz - twx;
	view->m[10] = 1.0f - (txx + tyy);
}

static void update_translation(mat4* view, const vec3* eye)
{
	view->m[12] =
		-view->m[0] * eye->x - view->m[4] * eye->y - view->m[8] * eye->z;
	view->m[13] =
		-view->m[1] * eye->x - view->m[5] * eye->y - view->m[9] * eye->z;
	view->m[14] =
		-view->m[2] * eye->x - view->m[6] * eye->y - view->m[10] * eye->z;
}

ce_camera* ce_camera_open(void)
{
	ce_camera* cam = ce_alloc(sizeof(ce_camera));
	if (NULL == cam) {
		return NULL;
	}
	cam->fov = 60.0f;
	cam->aspect = 1.0f;
	cam->near = 1.0f;
	cam->far = 500.0f;
	vec3_zero(&cam->eye);
	quat_identity(&cam->look);
	mat4_identity(&cam->view);
	cam->proj_changed = true;
	cam->eye_changed = true;
	cam->look_changed = true;
	return cam;
}

void ce_camera_close(ce_camera* cam)
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

vec3* ce_camera_get_eye(ce_camera* cam, vec3* eye)
{
	return vec3_copy(&cam->eye, eye);
}

vec3* ce_camera_get_forward(ce_camera* cam, vec3* forward)
{
	quat q;
	return vec3_rot(&VEC3_NEG_UNIT_Z, quat_conj(&cam->look, &q), forward);
}

vec3* ce_camera_get_up(ce_camera* cam, vec3* up)
{
	quat q;
	return vec3_rot(&VEC3_UNIT_Y, quat_conj(&cam->look, &q), up);
}

vec3* ce_camera_get_right(ce_camera* cam, vec3* right)
{
	quat q;
	return vec3_rot(&VEC3_UNIT_X, quat_conj(&cam->look, &q), right);
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

void ce_camera_set_eye(ce_camera* cam, const vec3* eye)
{
	vec3_copy(eye, &cam->eye);
	cam->eye_changed = true;
}

void ce_camera_set_look(ce_camera* cam, const quat* look)
{
	quat_copy(look, &cam->look);
	cam->look_changed = true;
}

void ce_camera_move(ce_camera* cam, float offset_x, float offset_z)
{
	vec3 forward, right;

	ce_camera_get_forward(cam, &forward);
	ce_camera_get_right(cam, &right);

	// Ignore pitch difference angle.
	forward.y = 0.0f;
	right.y = 0.0f;

	vec3_normalise(&forward, &forward);
	vec3_normalise(&right, &right);

	vec3_scale(&forward, offset_z, &forward);
	vec3_scale(&right, offset_x, &right);

	vec3_add(&cam->eye, &forward, &cam->eye);
	vec3_add(&cam->eye, &right, &cam->eye);

	cam->eye_changed = true;
}

void ce_camera_zoom(ce_camera* cam, float offset)
{
	vec3 forward;
	ce_camera_get_forward(cam, &forward);
	vec3_scale(&forward, offset, &forward);
	vec3_add(&cam->eye, &forward, &cam->eye);
	cam->eye_changed = true;
}

void ce_camera_yaw_pitch(ce_camera* cam, float psi, float theta)
{
	vec3 y;
	quat q, t;
	vec3_rot(&VEC3_UNIT_Y, &cam->look, &y);
	quat_mul(quat_init_polar(psi, &y, &q), &cam->look, &t);
	quat_mul(quat_init_polar(theta, &VEC3_UNIT_X, &q), &t, &cam->look);
	cam->look_changed = true;
}

void ce_camera_setup(ce_camera* cam)
{
	if (cam->proj_changed) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(cam->fov, cam->aspect, cam->near, cam->far);
		glMatrixMode(GL_MODELVIEW);
	}

	if (cam->look_changed) {
		update_rotation(&cam->view, &cam->look);
		update_translation(&cam->view, &cam->eye);
		cam->look_changed = false;
		cam->eye_changed = false;
	}

	if (cam->eye_changed) {
		update_translation(&cam->view, &cam->eye);
		cam->eye_changed = false;
	}

	glMultMatrixf(cam->view.m);
}
