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

#include "celib.h"
#include "cemath.h"
#include "cemat4.h"
#include "cealloc.h"
#include "cerendersystem.h"

struct ce_rendersystem {
	ce_mat4 view;
};

ce_rendersystem* ce_rendersystem_new(void)
{
	ce_rendersystem* rendersystem = ce_alloc(sizeof(ce_rendersystem));
	rendersystem->view = CE_MAT4_IDENTITY;
	return rendersystem;
}

void ce_rendersystem_del(ce_rendersystem* rendersystem)
{
	if (NULL != rendersystem) {
		ce_free(rendersystem, sizeof(ce_rendersystem));
	}
}

void ce_rendersystem_begin_render(ce_rendersystem* rendersystem,
									const ce_lightcfg* lightcfg)
{
	ce_unused(rendersystem);

	glClearColor(lightcfg->sky[0][0], lightcfg->sky[0][1],
				lightcfg->sky[0][2], lightcfg->sky[0][3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
}

void ce_rendersystem_end_render(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);
}

void ce_rendersystem_render_axes(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();

	glPopAttrib();
}

static void ce_rendersystem_update_projection(ce_rendersystem* rendersystem,
												ce_camera* camera)
{
	ce_unused(rendersystem);

	if (camera->proj_changed) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(camera->fov, camera->aspect, camera->near, camera->far);
		glMatrixMode(GL_MODELVIEW);
		camera->proj_changed = false;
	}
}

static void ce_rendersystem_update_rotation(ce_rendersystem* rendersystem,
													ce_camera* camera)
{
	if (camera->look_changed) {
		float tx  = 2.0f * camera->look.x;
		float ty  = 2.0f * camera->look.y;
		float tz  = 2.0f * camera->look.z;
		float twx = tx * camera->look.w;
		float twy = ty * camera->look.w;
		float twz = tz * camera->look.w;
		float txx = tx * camera->look.x;
		float txy = ty * camera->look.x;
		float txz = tz * camera->look.x;
		float tyy = ty * camera->look.y;
		float tyz = tz * camera->look.y;
		float tzz = tz * camera->look.z;

		rendersystem->view.m[0] = 1.0f - (tyy + tzz);
		rendersystem->view.m[1] = txy + twz;
		rendersystem->view.m[2] = txz - twy;

		rendersystem->view.m[4] = txy - twz;
		rendersystem->view.m[5] = 1.0f - (txx + tzz);
		rendersystem->view.m[6] = tyz + twx;

		rendersystem->view.m[8] = txz + twy;
		rendersystem->view.m[9] = tyz - twx;
		rendersystem->view.m[10] = 1.0f - (txx + tyy);

		camera->look_changed = false;
		camera->eye_changed = true;
	}
}

static void ce_rendersystem_update_translation(ce_rendersystem* rendersystem,
													ce_camera* camera)
{
	if (camera->eye_changed) {
		rendersystem->view.m[12] = -
			rendersystem->view.m[0] * camera->eye.x -
			rendersystem->view.m[4] * camera->eye.y -
			rendersystem->view.m[8] * camera->eye.z;

		rendersystem->view.m[13] = -
			rendersystem->view.m[1] * camera->eye.x -
			rendersystem->view.m[5] * camera->eye.y -
			rendersystem->view.m[9] * camera->eye.z;

		rendersystem->view.m[14] = -
			rendersystem->view.m[2] * camera->eye.x -
			rendersystem->view.m[6] * camera->eye.y -
			rendersystem->view.m[10] * camera->eye.z;

		camera->eye_changed = false;
	}
}

static void ce_rendersystem_update_view(ce_rendersystem* rendersystem,
											ce_camera* camera)
{
	ce_rendersystem_update_rotation(rendersystem, camera);
	ce_rendersystem_update_translation(rendersystem, camera);

	glMultMatrixf(rendersystem->view.m);
}

void ce_rendersystem_setup_camera(ce_rendersystem* rendersystem,
										ce_camera* camera)
{
	ce_rendersystem_update_projection(rendersystem, camera);
	ce_rendersystem_update_view(rendersystem, camera);
}

void ce_rendersystem_apply_transform(ce_rendersystem* rendersystem,
									const ce_vec3* translation,
									const ce_quat* rotation)
{
	ce_unused(rendersystem);

	glPushMatrix();

	glTranslatef(translation->x,
				translation->y,
				translation->z);

	ce_vec3 rotation_axis;
	float angle = ce_quat_to_angle_axis(rotation, &rotation_axis);

	glRotatef(ce_rad2deg(angle),
				rotation_axis.x,
				rotation_axis.y,
				rotation_axis.z);
}

void ce_rendersystem_discard_transform(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glPopMatrix();
}
