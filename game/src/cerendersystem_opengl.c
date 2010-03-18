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
}

void ce_rendersystem_end_render(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glFlush();
}

void ce_rendersystem_draw_axes(ce_rendersystem* rendersystem)
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

void ce_rendersystem_draw_wire_cube(ce_rendersystem* rendersystem,
									float size, const ce_color* color)
{
	ce_unused(rendersystem);

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glColor4f(color->r, color->g, color->b, color->a);

	glBegin(GL_QUADS);

	// face 1 front xy plane
	glVertex3f(-size, -size, size);
	glVertex3f( size, -size, size);
	glVertex3f( size,  size, size);
	glVertex3f(-size,  size, size);

	// face 2 right yz plane
	glVertex3f( size, -size,  size);
	glVertex3f( size, -size, -size);
	glVertex3f( size,  size, -size);
	glVertex3f( size,  size,  size);

	// face 3 back xy plane
	glVertex3f( size, -size, -size);
	glVertex3f(-size, -size, -size);
	glVertex3f(-size,  size, -size);
	glVertex3f( size,  size, -size);

	// face 4 left yz plane
	glVertex3f(-size, -size,  size);
	glVertex3f(-size, -size, -size);
	glVertex3f(-size,  size, -size);
	glVertex3f(-size,  size,  size);

	// face 5 top xz plane
	glVertex3f( size, size,  size);
	glVertex3f( size, size, -size);
	glVertex3f(-size, size, -size);
	glVertex3f(-size, size,  size);

	// face 6 bottom xz plane
	glVertex3f(-size, -size, -size);
	glVertex3f(-size, -size,  size);
	glVertex3f( size, -size,  size);
	glVertex3f( size, -size, -size);

	glEnd();

	glPopAttrib();
}

void ce_rendersystem_render_wire_aabb(ce_rendersystem* rendersystem,
												const ce_aabb* aabb,
												const ce_color* color)
{
	ce_rendersystem_apply_transform(rendersystem, &aabb->origin,
									&CE_QUAT_IDENTITY, &aabb->extents);
	ce_rendersystem_draw_wire_cube(rendersystem, 1.0f, color);
	ce_rendersystem_discard_transform(rendersystem);
}

void ce_rendersystem_render_wire_bbox(ce_rendersystem* rendersystem,
												const ce_color* color,
												const ce_bbox* bbox)
{
	ce_rendersystem_apply_transform(rendersystem, &bbox->aabb.origin,
									&bbox->axis, &bbox->aabb.extents);
	ce_rendersystem_draw_wire_cube(rendersystem, 1.0f, color);
	ce_rendersystem_discard_transform(rendersystem);
}

void ce_rendersystem_setup_camera(ce_rendersystem* rendersystem,
										ce_camera* camera)
{
	float tx  = 2.0f * camera->orientation.x;
	float ty  = 2.0f * camera->orientation.y;
	float tz  = 2.0f * camera->orientation.z;
	float twx = tx * camera->orientation.w;
	float twy = ty * camera->orientation.w;
	float twz = tz * camera->orientation.w;
	float txx = tx * camera->orientation.x;
	float txy = ty * camera->orientation.x;
	float txz = tz * camera->orientation.x;
	float tyy = ty * camera->orientation.y;
	float tyz = tz * camera->orientation.y;
	float tzz = tz * camera->orientation.z;

	rendersystem->view.m[0] = 1.0f - (tyy + tzz);
	rendersystem->view.m[1] = txy + twz;
	rendersystem->view.m[2] = txz - twy;

	rendersystem->view.m[4] = txy - twz;
	rendersystem->view.m[5] = 1.0f - (txx + tzz);
	rendersystem->view.m[6] = tyz + twx;

	rendersystem->view.m[8] = txz + twy;
	rendersystem->view.m[9] = tyz - twx;
	rendersystem->view.m[10] = 1.0f - (txx + tyy);

	rendersystem->view.m[12] = -
		rendersystem->view.m[0] * camera->position.x -
		rendersystem->view.m[4] * camera->position.y -
		rendersystem->view.m[8] * camera->position.z;

	rendersystem->view.m[13] = -
		rendersystem->view.m[1] * camera->position.x -
		rendersystem->view.m[5] * camera->position.y -
		rendersystem->view.m[9] * camera->position.z;

	rendersystem->view.m[14] = -
		rendersystem->view.m[2] * camera->position.x -
		rendersystem->view.m[6] * camera->position.y -
		rendersystem->view.m[10] * camera->position.z;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera->fov, camera->aspect, camera->near, camera->far);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(rendersystem->view.m);
}

void ce_rendersystem_apply_transform(ce_rendersystem* rendersystem,
									const ce_vec3* translation,
									const ce_quat* rotation,
									const ce_vec3* scaling)
{
	ce_unused(rendersystem);

	ce_vec3 axis;
	float angle = ce_quat_to_polar(rotation, &axis);

	glPushMatrix();

	glTranslatef(translation->x, translation->y, translation->z);
	glRotatef(ce_rad2deg(angle), axis.x, axis.y, axis.z);
	glScalef(scaling->x, scaling->y, scaling->z);
}

void ce_rendersystem_discard_transform(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glPopMatrix();
}
