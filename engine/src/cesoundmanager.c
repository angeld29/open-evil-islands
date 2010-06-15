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
#include "cealloc.h"
#include "celogging.h"
#include "cesoundmanager.h"

ce_soundmanager* ce_soundmanager_new(const char* path)
{
	ce_soundmanager* soundmanager = ce_alloc_zero(sizeof(ce_soundmanager));
	soundmanager->path = ce_string_new_str(path);
	soundmanager->soundinstances = ce_vector_new();
	ce_logging_write("sound manager: root path is '%s'", path);
	return soundmanager;
}

void ce_soundmanager_del(ce_soundmanager* soundmanager)
{
	if (NULL != soundmanager) {
		ce_vector_for_each(soundmanager->soundinstances, ce_soundinstance_del);
		ce_vector_del(soundmanager->soundinstances);
		ce_string_del(soundmanager->path);
		ce_free(soundmanager, sizeof(ce_soundmanager));
	}
}

void ce_soundmanager_advance(ce_soundmanager* soundmanager, float elapsed)
{
	ce_unused(elapsed);

	for (size_t i = 0; i < soundmanager->soundinstances->count; ++i) {
		ce_soundinstance* sound_instance = soundmanager->soundinstances->items[i];
		if (ce_soundinstance_is_stopped(sound_instance)) {
			ce_soundinstance_del(sound_instance);
			ce_vector_remove_unordered(soundmanager->soundinstances, i--);
		}
	}
}

static ce_memfile* ce_soundmanager_open(ce_soundmanager* soundmanager, const char* name)
{
	char path[soundmanager->path->length + strlen(name) + 16];
	const char* extensions[] = {"wav", "oga", "mp3", "ogv", "bik"};

	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
		snprintf(path, sizeof(path), "%s/%s.%s", soundmanager->path->str, name, extensions[i]);
		ce_memfile* memfile = ce_memfile_open_path(path);
		if (NULL != memfile) {
			return memfile;
		}
	}

	return NULL;
}

ce_sound_id ce_soundmanager_create(ce_soundmanager* soundmanager, const char* name)
{
	ce_memfile* memfile = ce_soundmanager_open(soundmanager, name);
	if (NULL == memfile) {
		return 0;
	}

	ce_soundresource* soundresource = ce_soundresource_new_builtin(memfile);
	if (NULL == soundresource) {
		ce_logging_error("sound manager: could not find decoder for '%s'", name);
		ce_memfile_close(memfile);
		return 0;
	}

	ce_soundinstance* soundinstance =
		ce_soundinstance_new(++soundmanager->last_sound_id, soundresource);
	if (NULL == soundinstance) {
		ce_logging_error("sound manager: could not create instance for '%s'", name);
		ce_soundresource_del(soundresource);
		return 0;
	}

	ce_vector_push_back(soundmanager->soundinstances, soundinstance);

	return soundinstance->sound_id;
}

ce_soundinstance* ce_soundmanager_find(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	for (size_t i = 0; i < soundmanager->soundinstances->count; ++i) {
		ce_soundinstance* sound_instance = soundmanager->soundinstances->items[i];
		if (sound_id == sound_instance->sound_id) {
			return sound_instance;
		}
	}
	return NULL;
}

bool ce_soundmanager_is_stopped(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(soundmanager, sound_id);
	if (NULL != soundinstance) {
		return ce_soundinstance_is_stopped(soundinstance);
	}
	return true;
}

void ce_soundmanager_play(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(soundmanager, sound_id);
	if (NULL != soundinstance) {
		ce_soundinstance_play(soundinstance);
	}
}

void ce_soundmanager_pause(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(soundmanager, sound_id);
	if (NULL != soundinstance) {
		ce_soundinstance_pause(soundinstance);
	}
}

void ce_soundmanager_stop(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(soundmanager, sound_id);
	if (NULL != soundinstance) {
		ce_soundinstance_stop(soundinstance);
	}
}

float ce_soundmanager_time(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(soundmanager, sound_id);
	if (NULL != soundinstance) {
		return ce_soundinstance_time(soundinstance);
	}
	return 0.0f;
}
