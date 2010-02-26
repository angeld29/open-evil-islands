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

#include <GL/gl.h>
#include <GL/glu.h>

#include "cecamera.h"

static void ce_camera_update_rotation(ce_mat4* view, const ce_quat* look)
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

static void ce_camera_update_translation(ce_mat4* view, const ce_vec3* eye)
{
	view->m[12] =
		-view->m[0] * eye->x - view->m[4] * eye->y - view->m[8] * eye->z;
	view->m[13] =
		-view->m[1] * eye->x - view->m[5] * eye->y - view->m[9] * eye->z;
	view->m[14] =
		-view->m[2] * eye->x - view->m[6] * eye->y - view->m[10] * eye->z;
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
		ce_camera_update_rotation(&cam->view, &cam->look);
		ce_camera_update_translation(&cam->view, &cam->eye);
		cam->look_changed = false;
		cam->eye_changed = false;
	}

	if (cam->eye_changed) {
		ce_camera_update_translation(&cam->view, &cam->eye);
		cam->eye_changed = false;
	}

	glMultMatrixf(cam->view.m);
}
