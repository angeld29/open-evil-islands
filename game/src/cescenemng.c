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

// TODO: to be unhardcoded...
#include <GL/glut.h>
#ifdef _WIN32
// fu... win32
#undef near
#undef far
#endif

#include "cegl.h"

#include "cemath.h"
#include "celogging.h"
#include "cealloc.h"
#include "cefrustum.h"
#include "cescenemng.h"

ce_scenemng* ce_scenemng_new(void)
{
	ce_scenemng* scenemng = ce_alloc_zero(sizeof(ce_scenemng));
	scenemng->scenenode = ce_scenenode_new(NULL);
	scenemng->rendersystem = ce_rendersystem_new();
	scenemng->renderqueue = ce_renderqueue_new();
	scenemng->timer = ce_timer_new();
	scenemng->fps = ce_fps_new();
	scenemng->camera = ce_camera_new();
	scenemng->font = ce_font_new(CE_FONT_TYPE_HELVETICA_18);
	scenemng->show_bboxes = false;
	scenemng->anm_fps = 15.0f;
	return scenemng;
}

void ce_scenemng_del(ce_scenemng* scenemng)
{
	if (NULL != scenemng) {
		ce_font_del(scenemng->font);
		ce_camera_del(scenemng->camera);
		ce_fps_del(scenemng->fps);
		ce_timer_del(scenemng->timer);
		ce_renderqueue_del(scenemng->renderqueue);
		ce_rendersystem_del(scenemng->rendersystem);
		ce_scenenode_del(scenemng->scenenode);
		ce_free(scenemng, sizeof(ce_scenemng));
	}
}

void ce_scenemng_advance(ce_scenemng* scenemng)
{
	ce_timer_advance(scenemng->timer);

	float elapsed = ce_timer_elapsed(scenemng->timer);

	ce_input_advance(elapsed);
	ce_fps_advance(scenemng->fps, elapsed);
}

void ce_scenemng_render_bboxes(ce_scenenode* scenenode)
{
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	glTranslatef(scenenode->world_bbox.aabb.origin.x,
				scenenode->world_bbox.aabb.origin.y,
				scenenode->world_bbox.aabb.origin.z);

	ce_vec3 xaxis, yaxis, zaxis, v;
	ce_quat_to_axes(&scenenode->world_bbox.axis, &xaxis, &yaxis, &zaxis);

	// TODO: to be unhardcoded...

#if 0
	glColor3f(0.0f, 0.0f, 1.0f);
	glutWireSphere(0.05f, 40, 40);

	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	ce_vec3_scale(&v, &xaxis, scenenode->world_bbox.aabb.extents.x);
	glVertex3f(v.x, v.y, v.z);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-v.x, -v.y, -v.z);
	glEnd();
	glPushMatrix();
	glTranslatef(v.x, v.y, v.z);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutWireSphere(0.05f, 40, 40);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-v.x, -v.y, -v.z);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutWireSphere(0.05f, 40, 40);
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	ce_vec3_scale(&v, &yaxis, scenenode->world_bbox.aabb.extents.y);
	glVertex3f(v.x, v.y, v.z);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-v.x, -v.y, -v.z);
	glEnd();
	glPushMatrix();
	glTranslatef(v.x, v.y, v.z);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutWireSphere(0.05f, 40, 40);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-v.x, -v.y, -v.z);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutWireSphere(0.05f, 40, 40);
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	ce_vec3_scale(&v, &zaxis, scenenode->world_bbox.aabb.extents.z);
	glVertex3f(v.x, v.y, v.z);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-v.x, -v.y, -v.z);
	glEnd();
	glPushMatrix();
	glTranslatef(v.x, v.y, v.z);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutWireSphere(0.05f, 40, 40);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-v.x, -v.y, -v.z);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutWireSphere(0.05f, 40, 40);
	glPopMatrix();
#endif

	ce_vec3_scale(&v, scenenode->world_bbox.aabb.extents.x, &xaxis);
	float xscale = ce_vec3_len(&v);
	ce_vec3_scale(&v, scenenode->world_bbox.aabb.extents.y, &yaxis);
	float yscale = ce_vec3_len(&v);
	ce_vec3_scale(&v, scenenode->world_bbox.aabb.extents.z, &zaxis);
	float zscale = ce_vec3_len(&v);
	float angle = ce_quat_to_polar(&scenenode->world_bbox.axis, &v);
	glRotatef(ce_rad2deg(angle), v.x, v.y, v.z);
	glScalef(xscale, yscale, zscale);
	glColor3f(0.0f, 0.0f, 1.0f);
	glutWireCube(2.0f);

	glPopMatrix();

	glPopAttrib();

	for (int i = 0; i < scenenode->childs->count; ++i) {
		ce_scenemng_render_bboxes(scenenode->childs->items[i]);
	}
}

void ce_scenemng_render(ce_scenemng* scenemng)
{
	// TODO: begin

	ce_rendersystem_setup_camera(scenemng->rendersystem, scenemng->camera);

	ce_rendersystem_render_axes(scenemng->rendersystem);

	ce_vec3 forward, right, up;
	ce_frustum frustum;

	ce_frustum_init(&frustum, scenemng->camera->fov,
		scenemng->camera->aspect, scenemng->camera->near,
		scenemng->camera->far, &scenemng->camera->position,
		ce_camera_get_forward(scenemng->camera, &forward),
		ce_camera_get_right(scenemng->camera, &right),
		ce_camera_get_up(scenemng->camera, &up));

	ce_renderqueue_clear(scenemng->renderqueue);
	ce_scenenode_update_cascade(scenemng->scenenode);
	ce_renderqueue_add_cascade(scenemng->renderqueue,
								scenemng->scenenode,
								&scenemng->camera->position,
								&frustum);
	ce_renderqueue_render(scenemng->renderqueue,
							scenemng->rendersystem);

	if (scenemng->show_bboxes) {
		ce_scenemng_render_bboxes(scenemng->scenenode);
	}

	// FIXME: hardcoded
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int width = viewport[2];
	int height = viewport[3];

	static char text[64];

	snprintf(text, sizeof(text),
			"smallobj allocated: %u b, max: %u b",
			ce_alloc_get_smallobj_allocated(),
			ce_alloc_get_smallobj_max_allocated());

	ce_font_render(scenemng->font, 10,
		height - 1 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, text);

	snprintf(text, sizeof(text),
			"smallobj overhead: %u b",
			ce_alloc_get_smallobj_overhead());

	ce_font_render(scenemng->font, 10,
		height - 2 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, text);

	snprintf(text, sizeof(text),
			"system allocated: %u b, max: %u b",
			ce_alloc_get_system_allocated(),
			ce_alloc_get_system_max_allocated());

	ce_font_render(scenemng->font, 10,
		height - 3 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, text);

	snprintf(text, sizeof(text), "%d scene nodes in frustum",
			scenemng->renderqueue->queued_scenenode_count);

	ce_font_render(scenemng->font, 10, 10, &CE_COLOR_RED, text);

	ce_font_render(scenemng->font,
		width - ce_font_get_width(scenemng->font, scenemng->fps->text) - 10,
		height - ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, scenemng->fps->text);

	const char* engine_text = "Powered by Cursed Earth Engine";

	ce_font_render(scenemng->font,
		width - ce_font_get_width(scenemng->font, engine_text) - 10, 10,
		&CE_COLOR_RED, engine_text);

	// TODO: end
	ce_gl_report_errors();
}
