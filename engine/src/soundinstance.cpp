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
#include <climits>
#include <vector>

#include "lib.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "soundmixer.hpp"
#include "soundinstance.hpp"

namespace cursedearth
{
    void ce_sound_instance_exec(ce_sound_instance* sound_instance)
    {
        while (!sound_instance->done) {
            //std::vector<char> buffer();
            switch (sound_instance->state) {
            case CE_SOUND_STATE_PLAYING:
                size = ce_sound_resource_read(sound_instance->sound_resource, buffer.data(), size);
                ce_sound_buffer_write(sound_instance->sound_buffer, buffer.data(), size);
                sound_instance->time = sound_instance->sound_resource->time;
                if (0 == size) {
                    ce_sound_instance_change_state(sound_instance, CE_SOUND_STATE_STOPPED);
                }
                break;
            case CE_SOUND_STATE_STOPPED:
                sound_instance->time = 0.0f;
                ce_sound_resource_reset(sound_instance->sound_resource);
                break;
            }
        }
    }

    ce_sound_instance* ce_sound_instance_new(ce_sound_object sound_object, ce_sound_resource* sound_resource)
    {
        ce_sound_instance* sound_instance = (ce_sound_instance*)ce_alloc_zero(sizeof(ce_sound_instance));
        sound_instance->sound_object = sound_object;
        sound_instance->sound_resource = sound_resource;
        sound_instance->sound_buffer = ce_sound_mixer_create_buffer();
        sound_instance->sound_buffer->sound_format = sound_resource->sound_format;
        sound_instance->thread = ce_thread_new((void(*)())ce_sound_instance_exec, sound_instance);
        return sound_instance;
    }

    void ce_sound_instance_del(ce_sound_instance* sound_instance)
    {
        if (NULL != sound_instance) {
            sound_instance->done = true;
            ce_thread_wait(sound_instance->thread);
            ce_thread_del(sound_instance->thread);
            ce_sound_mixer_destroy_buffer(sound_instance->sound_buffer);
            ce_sound_resource_del(sound_instance->sound_resource);
            ce_free(sound_instance, sizeof(ce_sound_instance));
        }
    }

    void ce_sound_instance_advance(ce_sound_instance*, float /*elapsed*/)
    {
    }

    void ce_sound_instance_change_state(ce_sound_instance* sound_instance, int state)
    {
        sound_instance->state = state;
    }
}
