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

#include <stdio.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "cegl.h"
#include "celib.h"
#include "cemath.h"
#include "cemat4.h"
#include "cealloc.h"
#include "cefont.h"
#include "cerendersystem.h"

struct ce_rendersystem {
	ce_mat4 view;
	ce_font* font;
};

ce_rendersystem* ce_rendersystem_new(void)
{
	ce_rendersystem* rendersystem = ce_alloc(sizeof(ce_rendersystem));
	rendersystem->view = CE_MAT4_IDENTITY;
	rendersystem->font = ce_font_new(CE_FONT_TYPE_HELVETICA_18);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return rendersystem;
}

void ce_rendersystem_del(ce_rendersystem* rendersystem)
{
	if (NULL != rendersystem) {
		ce_font_del(rendersystem->font);
		ce_free(rendersystem, sizeof(ce_rendersystem));
	}
}

void ce_rendersystem_begin_render(ce_rendersystem* rendersystem,
									const ce_color* clear_color)
{
	ce_unused(rendersystem);

	glClearColor(clear_color->r, clear_color->g,
				clear_color->b, clear_color->a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
}

void ce_rendersystem_end_render(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

#if 0
	if (ce_gl_query_feature(CE_GL_FEATURE_MEMINFO)) {
		const int n = 3;
		GLint params[4], viewport[4];
		GLenum pnames[] = { CE_GL_VBO_FREE_MEMORY,
							CE_GL_TEXTURE_FREE_MEMORY,
							CE_GL_RENDERBUFFER_FREE_MEMORY };
		const char* names[] = { "VBO", "TEX", "RB" };
		char buffer[32];

		glGetIntegerv(GL_VIEWPORT, viewport);

		for (int i = 0; i < n; ++i) {
			glGetIntegerv(pnames[i], params);
			snprintf(buffer, sizeof(buffer), "%s: %.3f MB free",
				names[i], params[0] / 1000.0f);
			ce_font_render(rendersystem->font, 10, viewport[3] -
				(i + 3) * ce_font_get_height(rendersystem->font) - 10,
				&CE_COLOR_RED, buffer);
		}
	}
#endif

	ce_gl_report_errors();

	glFlush();
}

void ce_rendersystem_draw_axes(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

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
}

void ce_rendersystem_draw_wire_cube(ce_rendersystem* rendersystem,
									float size, const ce_color* color)
{
	ce_unused(rendersystem);

	glColor4f(color->r, color->g, color->b, color->a);

	glBegin(GL_LINE_STRIP);

	// face 1 front xy plane
	glVertex3f( size, -size,  size);
	glVertex3f( size,  size,  size);
	glVertex3f(-size,  size,  size);
	glVertex3f(-size, -size,  size);

	// face 2 right yz plane
	glVertex3f( size, -size,  size);
	glVertex3f( size, -size, -size);
	glVertex3f( size,  size, -size);
	glVertex3f( size,  size,  size);

	// face 3 top xz plane
	glVertex3f( size,  size, -size);
	glVertex3f(-size,  size, -size);
	glVertex3f(-size,  size,  size);

	// face 4 left yz plane
	glVertex3f(-size, -size,  size);
	glVertex3f(-size, -size, -size);
	glVertex3f(-size,  size, -size);

	// face 5 back xy plane
	glVertex3f( size,  size, -size);
	glVertex3f( size, -size, -size);
	glVertex3f(-size, -size, -size);

	// face 6 bottom xz plane
	glVertex3f(-size, -size,  size);
	glVertex3f( size, -size,  size);
	glVertex3f( size, -size, -size);

	glEnd();
}

void ce_rendersystem_setup_viewport(ce_rendersystem* rendersystem,
										ce_viewport* viewport)
{
	ce_unused(rendersystem);

	glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
	glScissor(viewport->x, viewport->y, viewport->width, viewport->height);
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
	glMultMatrixf(rendersystem->view.m);
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

void ce_rendersystem_apply_material(ce_rendersystem* rendersystem,
										ce_material* material)
{
	ce_unused(rendersystem);

	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_LIGHTING);

	glMaterialfv(GL_FRONT, GL_AMBIENT, (float[]) { material->ambient.r,
													material->ambient.g,
													material->ambient.b,
													material->ambient.a });
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[]) { material->diffuse.r,
													material->diffuse.g,
													material->diffuse.b,
													material->diffuse.a });
	glMaterialfv(GL_FRONT, GL_SPECULAR, (float[]) { material->specular.r,
													material->specular.g,
													material->specular.b,
													material->specular.a });
	glMaterialfv(GL_FRONT, GL_EMISSION, (float[]) { material->emission.r,
													material->emission.g,
													material->emission.b,
													material->emission.a });
	glMaterialf(GL_FRONT, GL_SHININESS, material->shininess);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
		(GLint[]){ GL_MODULATE, GL_DECAL, GL_REPLACE }[material->mode]);

	if (material->alpha_test) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5f);
	}

	if (material->blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void ce_rendersystem_discard_material(ce_rendersystem* rendersystem,
										ce_material* material)
{
	ce_unused(rendersystem), ce_unused(material);

	glPopAttrib();
}
