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

#include <GL/gl.h>

#include "celogging.h"
#include "cealloc.h"
#include "cefrustum.h"
#include "cescenemng.h"

ce_scenemng* ce_scenemng_new(void)
{
	ce_scenemng* scenemng = ce_alloc_zero(sizeof(ce_scenemng));
	if (NULL == scenemng) {
		ce_logging_error("scenemng: could not allocate memory");
		return NULL;
	}

	if (NULL == (scenemng->rendqueue = ce_rendqueue_new()) ||
			NULL == (scenemng->timer = ce_timer_new()) ||
			NULL == (scenemng->fps = ce_fps_new()) ||
			NULL == (scenemng->camera = ce_camera_new()) ||
			NULL == (scenemng->terrains = ce_vector_new()) ||
			NULL == (scenemng->font = ce_font_new(CE_FONT_TYPE_HELVETICA_18))) {
		ce_scenemng_del(scenemng);
		return NULL;
	}

	return scenemng;
}

void ce_scenemng_del(ce_scenemng* scenemng)
{
	if (NULL != scenemng) {
		if (NULL != scenemng->terrains) {
			for (int i = 0, n = ce_vector_count(scenemng->terrains); i < n; ++i) {
				ce_terrain_del(ce_vector_at(scenemng->terrains, i));
			}
			ce_vector_del(scenemng->terrains);
		}
		ce_font_del(scenemng->font);
		ce_camera_del(scenemng->camera);
		ce_fps_del(scenemng->fps);
		ce_timer_del(scenemng->timer);
		ce_rendqueue_del(scenemng->rendqueue);
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

void ce_scenemng_render(ce_scenemng* scenemng)
{
	glLoadIdentity();
	ce_camera_setup(scenemng->camera);

	glEnable(GL_DEPTH_TEST);

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);
	glEnd();
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	glEnd();
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();

	glDisable(GL_DEPTH_TEST);

	ce_rendqueue_clear(scenemng->rendqueue);

	ce_vec3 eye, forward, right, up;
	ce_frustum frustum;

	ce_frustum_init(&frustum,
		ce_camera_get_fov(scenemng->camera),
		ce_camera_get_aspect(scenemng->camera),
		ce_camera_get_near(scenemng->camera),
		ce_camera_get_far(scenemng->camera),
		ce_camera_get_eye(scenemng->camera, &eye),
		ce_camera_get_forward(scenemng->camera, &forward),
		ce_camera_get_right(scenemng->camera, &right),
		ce_camera_get_up(scenemng->camera, &up));

	for (int i = 0, n = ce_vector_count(scenemng->terrains); i < n; ++i) {
		ce_terrain* terrain = ce_vector_at(scenemng->terrains, i);
		ce_rendqueue_add_rendlayer(scenemng->rendqueue,
									terrain->rendlayer,
									&eye, &frustum);
	}

	ce_rendqueue_render(scenemng->rendqueue);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int width = viewport[2];
	int height = viewport[3];

	const char* engine_text = "Powered by Cursed Earth Engine";

	ce_font_render(scenemng->font,
		width - ce_font_get_width(scenemng->font, engine_text) - 10, 10,
		&CE_COLOR_GREEN, engine_text);

	ce_font_render(scenemng->font,
		width - ce_font_get_width(scenemng->font, scenemng->fps->text) - 10,
		height - ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, scenemng->fps->text);

	static char alloc_text[64];

	snprintf(alloc_text, sizeof(alloc_text),
			"smallobj allocated: %u b, max: %u b",
			ce_alloc_get_smallobj_allocated(),
			ce_alloc_get_smallobj_max_allocated());

	ce_font_render(scenemng->font, 10,
		height - 1 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, alloc_text);

	snprintf(alloc_text, sizeof(alloc_text),
			"smallobj overhead: %u b",
			ce_alloc_get_smallobj_overhead());

	ce_font_render(scenemng->font, 10,
		height - 2 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, alloc_text);

	snprintf(alloc_text, sizeof(alloc_text),
			"system allocated: %u b, max: %u b",
			ce_alloc_get_system_allocated(),
			ce_alloc_get_system_max_allocated());

	ce_font_render(scenemng->font, 10,
		height - 3 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, alloc_text);
}

ce_terrain* ce_scenemng_load_zone(ce_scenemng* scenemng, const char* zone_name)
{
	ce_terrain* terrain = ce_terrain_new(zone_name);
	if (terrain) {
		ce_vector_push_back(scenemng->terrains, terrain);
	}
	return terrain;
}
