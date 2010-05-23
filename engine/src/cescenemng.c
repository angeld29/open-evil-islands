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
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cemath.h"
#include "celogging.h"
#include "cealloc.h"
#include "cemprhlp.h"
#include "cefrustum.h"
#include "cebytefmt.h"
#include "ceroot.h"
#include "cescenemng.h"

void ce_scenemng_renderwindow_resized(void* listener, int width, int height)
{
	ce_scenemng* scenemng = listener;
	ce_viewport_set_rect(scenemng->viewport, 0, 0, width, height);
	ce_camera_set_aspect(scenemng->camera, (float)width / height);
}

static void ce_scenemng_figproto_created(void* listener, ce_figproto* figproto)
{
	ce_scenemng* scenemng = listener;
	ce_figproto_accept_renderqueue(figproto, scenemng->renderqueue);
}

ce_scenemng* ce_scenemng_new(const char* ei_path)
{
	ce_logging_write("scenemng: root path: '%s'", ei_path);

	ce_scenemng* scenemng = ce_alloc_zero(sizeof(ce_scenemng));
	scenemng->scenenode = ce_scenenode_new(NULL);
	scenemng->figmng = ce_figmng_new();
	scenemng->renderqueue = ce_renderqueue_new();
	scenemng->viewport = ce_viewport_new();
	scenemng->camera = ce_camera_new();
	scenemng->fps = ce_fps_new();
	scenemng->font = ce_font_new("fonts/evilislands.ttf", 24);
	scenemng->listeners = ce_vector_new();

	scenemng->input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
	scenemng->move_left_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowLeft");
	scenemng->move_up_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowUp");
	scenemng->move_right_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowRight");
	scenemng->move_down_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowDown");
	scenemng->zoom_in_event = ce_input_supply_shortcut(scenemng->input_supply, "WheelUp");
	scenemng->zoom_out_event = ce_input_supply_shortcut(scenemng->input_supply, "WheelDown");
	scenemng->rotate_on_event = ce_input_supply_shortcut(scenemng->input_supply, "MouseRight");

	scenemng->renderwindow_listener = (ce_renderwindow_listener)
		{.resized = ce_scenemng_renderwindow_resized, .listener = scenemng};
	scenemng->figmng_listener = (ce_figmng_listener)
		{.figproto_created = ce_scenemng_figproto_created, .listener = scenemng};

	ce_renderwindow_add_listener(ce_root.renderwindow, &scenemng->renderwindow_listener);
	ce_figmng_add_listener(scenemng->figmng, &scenemng->figmng_listener);

	char path[strlen(ei_path) + 32];

	snprintf(path, sizeof(path), "%s/Textures", ei_path);
	scenemng->texmng = ce_texmng_new(path);

	const char* texture_resources[] = { "textures", "redress", "menus" };
	for (int i = 0, n = sizeof(texture_resources) /
						sizeof(texture_resources[0]); i < n; ++i) {
		snprintf(path, sizeof(path), "%s/Res/%s.res", ei_path, texture_resources[i]);
		ce_texmng_register_resource(scenemng->texmng, path);
	}

	snprintf(path, sizeof(path), "%s/Maps", ei_path);
	scenemng->mprmng = ce_mprmng_new(path);

	const char* figure_resources[] = { "figures", "menus" };
	for (int i = 0, n = sizeof(figure_resources) /
						sizeof(figure_resources[0]); i < n; ++i) {
		snprintf(path, sizeof(path), "%s/Res/%s.res", ei_path, figure_resources[i]);
		ce_figmng_register_resource(scenemng->figmng, path);
	}

	return scenemng;
}

void ce_scenemng_del(ce_scenemng* scenemng)
{
	if (NULL != scenemng) {
		ce_input_supply_del(scenemng->input_supply);
		ce_vector_del(scenemng->listeners);
		ce_font_del(scenemng->font);
		ce_fps_del(scenemng->fps);
		ce_camera_del(scenemng->camera);
		ce_viewport_del(scenemng->viewport);
		ce_renderqueue_del(scenemng->renderqueue);
		ce_figmng_del(scenemng->figmng);
		ce_terrain_del(scenemng->terrain);
		ce_mprmng_del(scenemng->mprmng);
		ce_texmng_del(scenemng->texmng);
		ce_scenenode_del(scenemng->scenenode);
		ce_free(scenemng, sizeof(ce_scenemng));
	}
}

void ce_scenemng_add_listener(ce_scenemng* scenemng,
								ce_scenemng_listener* listener)
{
	ce_vector_push_back(scenemng->listeners, listener);
}

void ce_scenemng_advance(ce_scenemng* scenemng, float elapsed)
{
	ce_fps_advance(scenemng->fps, elapsed);
	ce_input_supply_advance(scenemng->input_supply, elapsed);

	if (scenemng->move_left_event->triggered) {
		ce_camera_move(scenemng->camera, -10.0f * elapsed, 0.0f);
	}

	if (scenemng->move_up_event->triggered) {
		ce_camera_move(scenemng->camera, 0.0f, 10.0f * elapsed);
	}

	if (scenemng->move_right_event->triggered) {
		ce_camera_move(scenemng->camera, 10.0f * elapsed, 0.0f);
	}

	if (scenemng->move_down_event->triggered) {
		ce_camera_move(scenemng->camera, 0.0f, -10.0f * elapsed);
	}

	if (scenemng->zoom_in_event->triggered) {
		ce_camera_zoom(scenemng->camera, 5.0f);
	}

	if (scenemng->zoom_out_event->triggered) {
		ce_camera_zoom(scenemng->camera, -5.0f);
	}

	if (scenemng->rotate_on_event->triggered) {
		ce_camera_yaw_pitch(scenemng->camera,
			ce_deg2rad(-0.25f * scenemng->input_supply->context->pointer_offset.x),
			ce_deg2rad(-0.25f * scenemng->input_supply->context->pointer_offset.y));
	}

	for (int i = 0; i < scenemng->listeners->count; ++i) {
		ce_scenemng_listener* listener = scenemng->listeners->items[i];
		if (NULL != listener->advance) {
			(*listener->advance)(listener->listener, elapsed);
		}
	}
}

void ce_scenemng_render(ce_scenemng* scenemng)
{
	ce_rendersystem_begin_render(ce_root.rendersystem, &CE_COLOR_WHITE);

	ce_rendersystem_setup_viewport(ce_root.rendersystem, scenemng->viewport);
	ce_rendersystem_setup_camera(ce_root.rendersystem, scenemng->camera);

	if (ce_root.show_axes) {
		ce_rendersystem_draw_axes(ce_root.rendersystem);
	}

	ce_renderqueue_render(scenemng->renderqueue, ce_root.rendersystem);
	ce_renderqueue_clear(scenemng->renderqueue);

	if (scenemng->scenenode_force_update) {
		// big changes of the scene node tree - force update
		ce_scenenode_update_force_cascade(scenemng->scenenode);
		scenemng->scenenode_force_update = false;
	} else {
		ce_vec3 forward, right, up;
		ce_frustum frustum;

		ce_frustum_init(&frustum, scenemng->camera->fov,
			scenemng->camera->aspect, scenemng->camera->near,
			scenemng->camera->far, &scenemng->camera->position,
			ce_camera_get_forward(scenemng->camera, &forward),
			ce_camera_get_right(scenemng->camera, &right),
			ce_camera_get_up(scenemng->camera, &up));

		ce_rendersystem_begin_occlusion_test(ce_root.rendersystem);
		ce_scenenode_update_cascade(scenemng->scenenode, &frustum);
		ce_rendersystem_end_occlusion_test(ce_root.rendersystem);
	}

	if (ce_root.show_bboxes) {
		ce_rendersystem_apply_color(ce_root.rendersystem, &CE_COLOR_BLUE);
		ce_scenenode_draw_bboxes_cascade(scenemng->scenenode);
	}

#if 0
#ifndef NDEBUG
	char text[128], bytefmt_text[64], bytefmt_text2[64], bytefmt_text3[64];
	snprintf(text, sizeof(text),
			"smallobj %s, max %s, overhead %s",
			ce_bytefmt_detail(bytefmt_text, sizeof(bytefmt_text),
								ce_alloc_get_smallobj_allocated()),
			ce_bytefmt_detail(bytefmt_text2, sizeof(bytefmt_text2),
								ce_alloc_get_smallobj_max_allocated()),
			ce_bytefmt_detail(bytefmt_text3, sizeof(bytefmt_text3),
								ce_alloc_get_smallobj_overhead()));

	ce_font_render(scenemng->font, 10, scenemng->viewport->height - 1 *
		ce_font_get_height(scenemng->font) - 10, &CE_COLOR_RED, text);

	snprintf(text, sizeof(text),
			"system %s, max %s",
			ce_bytefmt_detail(bytefmt_text, sizeof(bytefmt_text),
								ce_alloc_get_system_allocated()),
			ce_bytefmt_detail(bytefmt_text2, sizeof(bytefmt_text2),
								ce_alloc_get_system_max_allocated()));

	ce_font_render(scenemng->font, 10, scenemng->viewport->height - 2 *
		ce_font_get_height(scenemng->font) - 10, &CE_COLOR_RED, text);
#endif
#endif

#ifndef NDEBUG
	char scenenode_text[8];
	snprintf(scenenode_text, sizeof(scenenode_text), "%d",
		ce_scenenode_count_visible_cascade(scenemng->scenenode));

	ce_font_render(scenemng->font, 10, 10, &CE_COLOR_GRAY, scenenode_text);
#endif

	if (ce_root.show_fps) {
		ce_font_render(scenemng->font, scenemng->viewport->width -
			ce_font_get_width(scenemng->font, scenemng->fps->text) - 10,
			scenemng->viewport->height - ce_font_get_height(scenemng->font) - 10,
			&CE_COLOR_INDIGO, scenemng->fps->text);
	}

	const char* engine_text = "Powered by Cursed Earth Engine";

	ce_font_render(scenemng->font, scenemng->viewport->width -
		ce_font_get_width(scenemng->font, engine_text) - 10, 10,
		&CE_COLOR_RED, engine_text);

	for (int i = 0; i < scenemng->listeners->count; ++i) {
		ce_scenemng_listener* listener = scenemng->listeners->items[i];
		if (NULL != listener->render) {
			(*listener->render)(listener->listener);
		}
	}

	ce_rendersystem_end_render(ce_root.rendersystem);
}

ce_terrain* ce_scenemng_create_terrain(ce_scenemng* scenemng,
										const char* name,
										const ce_vec3* position,
										const ce_quat* orientation,
										ce_scenenode* scenenode)
{
	ce_mprfile* mprfile = ce_mprmng_open_mprfile(scenemng->mprmng, name);
	if (NULL == mprfile) {
		return NULL;
	}

	if (NULL == scenenode) {
		scenenode = scenemng->scenenode;
	}

	ce_terrain_del(scenemng->terrain);
	scenemng->terrain = ce_terrain_new(mprfile, scenemng->texmng,
		scenemng->renderqueue, position, orientation, scenenode);

	scenemng->scenenode_force_update = true;

	return scenemng->terrain;
}

ce_figentity*
ce_scenemng_create_figentity(ce_scenemng* scenemng,
							const char* name,
							const ce_complection* complection,
							const ce_vec3* position,
							const ce_quat* orientation,
							int texture_count,
							const char* texture_names[],
							ce_scenenode* scenenode)
{
	ce_texture* textures[texture_count];

	for (int i = 0; i < texture_count; ++i) {
		textures[i] = ce_texmng_get(scenemng->texmng, texture_names[i]);
		if (NULL == textures[i]) {
			ce_logging_error("scenemng: could not load texture "
				"'%s' for figentity '%s'", texture_names[i], name);
			return NULL;
		}
	}

	if (NULL == scenenode) {
		scenenode = scenemng->scenenode;
		if (NULL != scenemng->terrain) {
			scenenode = ce_terrain_find_scenenode(scenemng->terrain,
												position->x, position->z);
		}
	}

	ce_figentity* figentity =
		ce_figmng_create_figentity(scenemng->figmng, name,
									complection, position,
									orientation, texture_count,
									textures, scenenode);

	scenemng->scenenode_force_update = true;

	return figentity;
}

ce_figentity*
ce_scenemng_create_figentity_mobobject(ce_scenemng* scenemng,
									const ce_mobobject_object* mobobject)
{
	ce_vec3 position = mobobject->position;
	ce_fswap(&position.y, &position.z);
	if (NULL != scenemng->terrain) {
		position.y += ce_mprhlp_get_height(scenemng->terrain->mprfile,
											position.x, position.z);
	}
	// yeah! it's a real hard-code :) move creatures up
	if (50 == mobobject->type ||
			51 == mobobject->type ||
			52 == mobobject->type) {
		position.y += 1.0f;
	}
	position.z = -position.z; // FIXME: opengl's hardcode

	ce_quat orientation, q;
	ce_quat_init_polar(&q, ce_deg2rad(-90.0f), &CE_VEC3_UNIT_X);
	ce_quat_mul(&orientation, &q, &mobobject->rotation);

	const char* texture_names[] = { mobobject->primary_texture->str,
									mobobject->secondary_texture->str };

	return ce_scenemng_create_figentity(scenemng,
										mobobject->model_name->str,
										&mobobject->complection,
										&position, &orientation,
										2, texture_names, NULL);
}

void ce_scenemng_remove_figentity(ce_scenemng* scenemng, ce_figentity* figentity)
{
	ce_figmng_remove_figentity(scenemng->figmng, figentity);
}

void ce_scenemng_load_mobfile(ce_scenemng* scenemng,
								const ce_mobfile* mobfile)
{
	ce_logging_write("scenemng: loading mob '%s'...", mobfile->name->str);
	for (int i = 0; i < mobfile->objects->count; ++i) {
		const ce_mobobject_object* mobobject = mobfile->objects->items[i];
		ce_scenemng_create_figentity_mobobject(scenemng, mobobject);
	}
}
