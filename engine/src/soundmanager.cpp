/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>
#include <cstring>
#include <vector>

#include "path.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "resfile.hpp"
#include "resball.hpp"
#include "event.hpp"
#include "optionmanager.hpp"
#include "soundmanager.hpp"

namespace cursedearth
{
    typedef struct {
        ce_hash_key hash_key;
        ce_sound_bundle sound_bundle;
        char name[256];
    } ce_sound_event;

    struct ce_sound_manager* ce_sound_manager;

    const char* ce_sound_dirs[] = {"Stream", "Movies", NULL};
    const char* ce_sound_exts[] = {".wav", ".oga", ".ogv", ".ogg", ".mp3", ".bik", ".flac", NULL};
    const char* ce_sound_resource_dirs[] = {"Res", NULL};
    const char* ce_sound_resource_exts[] = {".res", NULL};
    const char* ce_sound_resource_names[] = {"sfx", "speech", NULL};

    void ce_sound_manager_create_instance(ce_event* event)
    {
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;
        ce_mem_file* mem_file = NULL;

        for (size_t i = 0; i < ce_sound_manager->res_files->count; ++i) {
            ce_res_file* res_file = (ce_res_file*)ce_sound_manager->res_files->items[i];
            if (NULL != (mem_file = ce_res_ball_extract_mem_file_by_name(res_file, sound_event->name))) {
                break;
            }
        }

        if (NULL == mem_file) {
            std::vector<char> path(ce_option_manager->ei_path->length + strlen(sound_event->name) + 32);
            if (NULL == ce_path_find_special1(path.data(), path.size(), ce_option_manager->ei_path->str, sound_event->name, ce_sound_dirs, ce_sound_exts)) {
                ce_logging_error("sound manager: could not find sound `%s'", sound_event->name);
                return;
            }

            mem_file = ce_mem_file_new_path(path.data());
            if (NULL == mem_file) {
                ce_logging_error("sound manager: could not open file `%s'", path.data());
                return;
            }
        }

        ce_sound_resource* sound_resource = ce_sound_resource_new(mem_file);
        if (NULL == sound_resource) {
            ce_logging_error("sound manager: could not find decoder for `%s'", sound_event->name);
            ce_mem_file_del(mem_file);
            return;
        }

        ce_sound_instance* sound_instance = ce_sound_instance_new(sound_resource);
        if (NULL == sound_instance) {
            ce_logging_error("sound manager: could not create instance '%s'", sound_event->name);
            ce_sound_resource_del(sound_resource);
            return;
        }

        ce_hash_insert(ce_sound_manager->sound_instances, sound_event->hash_key, sound_instance);
    }

    void ce_sound_manager_remove_instance(ce_event* event)
    {
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;
        ce_hash_remove(ce_sound_manager->sound_instances, sound_event->hash_key);
    }

    void ce_sound_manager_state_instance(ce_event* event)
    {
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;
        ce_sound_instance* sound_instance = (ce_sound_instance*)ce_hash_find(ce_sound_manager->sound_instances, sound_event->hash_key);
        if (NULL != sound_instance) {
            ce_sound_instance_change_state(sound_instance, sound_event->sound_bundle.state);
        }
    }

    ce_res_file* ce_sound_manager_open_resource(const char* name)
    {
        std::vector<char> path(ce_option_manager->ei_path->length + 32);
        ce_res_file* res_file = NULL;

        if (NULL != ce_path_find_special1(path.data(), path.size(), ce_option_manager->ei_path->str,
                name, ce_sound_resource_dirs, ce_sound_resource_exts) && NULL != (res_file = ce_res_file_new_path(path.data()))) {
            ce_logging_write("sound manager: loading `%s'... ok", path.data());
        } else {
            ce_logging_error("sound manager: loading `%s'... failed", path.data());
        }

        return res_file;
    }

    void ce_sound_manager_query_bundle(ce_event* event)
    {
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;
        ce_sound_bundle* sound_bundle = (ce_sound_bundle*)ce_hash_find(ce_sound_manager->sound_bundles, sound_event->hash_key);
        if (NULL != sound_bundle) {
            *sound_bundle = sound_event->sound_bundle;
        }
    }

    void ce_sound_manager_advance_instance(ce_sound_instance* sound_instance, float* elapsed)
    {
        ce_sound_instance_advance(sound_instance, *elapsed);
    }

    void ce_sound_manager_query_instance(ce_hash_key hash_key)
    {
        ce_sound_instance* sound_instance = (ce_sound_instance*)ce_hash_find(ce_sound_manager->sound_instances, hash_key);
        if (NULL != sound_instance) {
            ce_event* event = ce_event_new(ce_sound_manager_query_bundle, sizeof(ce_sound_event));
            ce_sound_event* sound_event = (ce_sound_event*)event->impl;
            sound_event->hash_key = hash_key;
            sound_event->sound_bundle = sound_instance->sound_bundle;
            ce_event_manager_post_event(ce_sound_manager->thread_id, event);
        }
    }

    void ce_sound_manager_idle(ce_event*)
    {
        float elapsed = ce_timer_advance(ce_sound_manager->timer);
        ce_hash_for_each_arg1(ce_sound_manager->sound_instances, (void(*)())ce_sound_manager_advance_instance, &elapsed);
        ce_hash_for_each_key(ce_sound_manager->sound_instances, ce_sound_manager_query_instance);
    }

    void ce_sound_manager_exec(void*)
    {
        std::vector<char> path(ce_option_manager->ei_path->length + 16);
        for (size_t i = 0; NULL != ce_sound_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), ce_option_manager->ei_path->str, ce_sound_dirs[i], NULL);
            ce_logging_write("sound manager: using path `%s'", path.data());
        }

        for (size_t i = 0; NULL != ce_sound_resource_names[i]; ++i) {
            ce_res_file* res_file = ce_sound_manager_open_resource(ce_sound_resource_names[i]);
            if (NULL != res_file) {
                ce_vector_push_back(ce_sound_manager->res_files, res_file);
            }
        }

        ce_timer_start(ce_sound_manager->timer);
        ce_thread_exec(ce_sound_manager->thread);
    }

    void ce_sound_manager_init(void)
    {
        ce_sound_manager = (struct ce_sound_manager*)ce_alloc_zero(sizeof(struct ce_sound_manager));
        ce_sound_manager->thread_id = ce_thread_self();
        ce_sound_manager->res_files = ce_vector_new();
        ce_sound_manager->sound_instances = ce_hash_new(32, (void(*)())ce_sound_instance_del);
        ce_sound_manager->sound_bundles = ce_hash_new(32, (void(*)())ce_sound_bundle_del);
        ce_sound_manager->timer = ce_timer_new();
        ce_sound_manager->thread = ce_thread_new((void(*)())ce_sound_manager_exec, NULL);
    }

    void ce_sound_manager_term(void)
    {
        if (NULL != ce_sound_manager) {
            ce_thread_exit_wait_del(ce_sound_manager->thread);
            ce_timer_del(ce_sound_manager->timer);
            ce_hash_del(ce_sound_manager->sound_bundles);
            ce_hash_del(ce_sound_manager->sound_instances);
            ce_vector_for_each(ce_sound_manager->res_files, (void(*)(void*))ce_res_file_del);
            ce_vector_del(ce_sound_manager->res_files);
            ce_free(ce_sound_manager, sizeof(struct ce_sound_manager));
        }
    }

    void ce_sound_manager_advance(float /*elapsed*/)
    {
        ce_event_manager_post_call(ce_thread_get_id(ce_sound_manager->thread), ce_sound_manager_idle);
    }

    ce_hash_key ce_sound_manager_create_object(const char* name)
    {
        size_t length = strlen(name);
        size_t size = sizeof(ce_sound_event);

        ce_event* event = ce_event_new(ce_sound_manager_create_instance, size);
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;

        sound_event->hash_key = ++ce_sound_manager->last_hash_key;
        memcpy(sound_event->name, name, length);

        ce_event_manager_post_event(ce_thread_get_id(ce_sound_manager->thread), event);
        ce_hash_insert(ce_sound_manager->sound_bundles, sound_event->hash_key, ce_sound_bundle_new());

        return sound_event->hash_key;
    }

    void ce_sound_manager_remove_object(ce_hash_key hash_key)
    {
        ce_event* event = ce_event_new(ce_sound_manager_remove_instance, sizeof(ce_sound_event));
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;
        sound_event->hash_key = hash_key;
        ce_event_manager_post_event(ce_thread_get_id(ce_sound_manager->thread), event);
        ce_hash_remove(ce_sound_manager->sound_bundles, hash_key);
    }

    void ce_sound_manager_state_object(ce_hash_key hash_key, int state)
    {
        ce_event* event = ce_event_new(ce_sound_manager_state_instance, sizeof(ce_sound_event));
        ce_sound_event* sound_event = (ce_sound_event*)event->impl;
        sound_event->hash_key = hash_key;
        sound_event->sound_bundle.state = state;
        ce_event_manager_post_event(ce_thread_get_id(ce_sound_manager->thread), event);
    }
}
