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

// FIXME: hardcoded
#include <GL/gl.h>

#include "cemath.h"
#include "celogging.h"
#include "cealloc.h"
#include "cefrustum.h"
#include "ceformat.h"
#include "cescenemng.h"

ce_scenemng* ce_scenemng_new(void)
{
	ce_scenemng* scenemng = ce_alloc(sizeof(ce_scenemng));
	scenemng->scenenode = ce_scenenode_new(NULL);
	scenemng->rendersystem = ce_rendersystem_new();
	scenemng->renderqueue = ce_renderqueue_new();
	scenemng->timer = ce_timer_new();
	scenemng->fps = ce_fps_new();
	scenemng->camera = ce_camera_new();
	scenemng->font = ce_font_new(CE_FONT_TYPE_HELVETICA_18);
	scenemng->show_axes = true;
	scenemng->show_bboxes = false;
	scenemng->comprehensive_bbox_only = true;
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

void ce_scenemng_render(ce_scenemng* scenemng)
{
	ce_rendersystem_begin_render(scenemng->rendersystem, &CE_COLOR_WHITE);

	ce_rendersystem_setup_camera(scenemng->rendersystem, scenemng->camera);

	if (scenemng->show_axes) {
		ce_rendersystem_draw_axes(scenemng->rendersystem);
	}

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
		ce_scenenode_draw_bbox_cascade(scenemng->scenenode,
										scenemng->rendersystem,
										scenemng->comprehensive_bbox_only);
	}

	// FIXME: hardcoded
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int width = viewport[2];
	int height = viewport[3];

	char text[128], bytefmt_text[64], bytefmt_text2[64], bytefmt_text3[64];

	snprintf(text, sizeof(text),
			"smallobj %s, max %s, overhead %s",
			ce_format_byte_detail(bytefmt_text, sizeof(bytefmt_text),
									ce_alloc_get_smallobj_allocated()),
			ce_format_byte_detail(bytefmt_text2, sizeof(bytefmt_text2),
									ce_alloc_get_smallobj_max_allocated()),
			ce_format_byte_detail(bytefmt_text3, sizeof(bytefmt_text3),
									ce_alloc_get_smallobj_overhead()));

	ce_font_render(scenemng->font, 10,
		height - 1 * ce_font_get_height(scenemng->font) - 10,
		&CE_COLOR_RED, text);

	snprintf(text, sizeof(text),
			"system %s, max %s",
			ce_format_byte_detail(bytefmt_text, sizeof(bytefmt_text),
									ce_alloc_get_system_allocated()),
			ce_format_byte_detail(bytefmt_text2, sizeof(bytefmt_text2),
									ce_alloc_get_system_max_allocated()));

	ce_font_render(scenemng->font, 10,
		height - 2 * ce_font_get_height(scenemng->font) - 10,
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

	ce_rendersystem_end_render(scenemng->rendersystem);
}
