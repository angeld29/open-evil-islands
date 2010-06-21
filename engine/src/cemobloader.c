/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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
#include "cealloc.h"
#include "celogging.h"
#include "cethread.h"
#include "ceeventmanager.h"
#include "cerendersystem.h"
#include "cefiguremanager.h"
#include "cemobmanager.h"
#include "cemobloader.h"

struct ce_mob_loader* ce_mob_loader;

typedef struct {
	ce_mob_task* mob_task;
	const char* model_name;
	ce_complection complection;
	ce_vec3 position;
	ce_quat orientation;
	const char* textures[3]; // NULL-terminated
	const char* parts[]; // NULL-terminated
} ce_mob_object_event;

static void ce_mob_object_event_react(ce_event* event)
{
	ce_mob_object_event* mob_event = (ce_mob_object_event*)event->impl;
	ce_mob_task* mob_task = mob_event->mob_task;

	ce_figure_manager_create_entity(mob_event->model_name,
		&mob_event->complection, &mob_event->position,
		&mob_event->orientation, mob_event->parts, mob_event->textures);

	if (++mob_task->processed_event_count == mob_task->posted_event_count) {
		ce_logging_info("mob task: done loading '%s'", mob_task->name->str);

		ce_vector_remove_all(ce_mob_loader->mob_tasks, mob_task);
		ce_mob_task_del(mob_task);

		if (++ce_mob_loader->completed_job_count == ce_mob_loader->queued_job_count) {
			// do not accumulate counters
			ce_mob_loader->completed_job_count = 0;
			ce_mob_loader->queued_job_count = 0;
		}
	}
}

static void ce_mob_task_exec(ce_mob_task* mob_task)
{
	mob_task->mob_file = ce_mob_manager_open(mob_task->name->str);
	if (NULL == mob_task->mob_file) {
		ce_logging_error("mob task: could not load '%s'", mob_task->name->str);
		return;
	}

	mob_task->posted_event_count = mob_task->mob_file->objects->count;

	ce_logging_info("mob task: loading '%s'...", mob_task->name->str);
	ce_logging_info("mob task: %zu events posted", mob_task->posted_event_count);

	for (size_t i = 0; i < mob_task->mob_file->objects->count; ++i) {
		ce_mobobject_object* mob_object = mob_task->mob_file->objects->items[i];

		ce_event* event = ce_event_new(ce_mob_object_event_react,
			sizeof(ce_mob_object_event) +
			sizeof(const char*) * (mob_object->parts->count + 1));

		ce_mob_object_event* mob_object_event = (ce_mob_object_event*)event->impl;

		mob_object_event->mob_task = mob_task;
		mob_object_event->model_name = mob_object->model_name->str;
		mob_object_event->complection = mob_object->complection;
		mob_object_event->position = mob_object->position;

		ce_swap_temp(float, &mob_object_event->position.y,
							&mob_object_event->position.z);

		// FIXME: yeah! it's a real hard-code :) move creatures up
		if (50 == mob_object->type || 51 == mob_object->type || 52 == mob_object->type) {
			mob_object_event->position.y += 1.0f;
		}

		// FIXME: GL's hard-code
		mob_object_event->position.z = -mob_object_event->position.z;

		ce_quat quat;
		ce_quat_init_polar(&quat, ce_deg2rad(-90.0f), &CE_VEC3_UNIT_X);
		ce_quat_mul(&mob_object_event->orientation, &quat, &mob_object->rotation);

		for (size_t j = 0; j < mob_object->parts->count; ++j) {
			ce_string* part = mob_object->parts->items[j];
			mob_object_event->parts[j] = part->str;
		}

		mob_object_event->textures[0] = mob_object->primary_texture->str;
		mob_object_event->textures[1] = mob_object->secondary_texture->str;

		ce_event_manager_post_event(ce_render_system->thread_id, event);
	}
}

ce_mob_task* ce_mob_task_new(const char* name)
{
	ce_mob_task* mob_task = ce_alloc_zero(sizeof(ce_mob_task));
	mob_task->name = ce_string_new_str(name);
	ce_thread_pool_enqueue(ce_mob_task_exec, mob_task);
	return mob_task;
}

void ce_mob_task_del(ce_mob_task* mob_task)
{
	if (NULL != mob_task) {
		ce_mobfile_close(mob_task->mob_file);
		ce_string_del(mob_task->name);
		ce_free(mob_task, sizeof(ce_mob_task));
	}
}

void ce_mob_loader_init(void)
{
	ce_mob_loader = ce_alloc_zero(sizeof(struct ce_mob_loader));
	ce_mob_loader->mob_tasks = ce_vector_new_reserved(2);
}

void ce_mob_loader_term(void)
{
	if (NULL != ce_mob_loader) {
		ce_vector_for_each(ce_mob_loader->mob_tasks, ce_mob_task_del);
		ce_vector_del(ce_mob_loader->mob_tasks);
		ce_free(ce_mob_loader, sizeof(struct ce_mob_loader));
	}
}

void ce_mob_loader_load_mob(const char* name)
{
	++ce_mob_loader->queued_job_count;
	ce_vector_push_back(ce_mob_loader->mob_tasks, ce_mob_task_new(name));
	ce_logging_info("mob loader: '%s' queued", name);
}