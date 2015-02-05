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

#ifndef CE_SOUNDINSTANCE_HPP
#define CE_SOUNDINSTANCE_HPP

#include <atomic>

#include "thread.hpp"
#include "soundobject.hpp"
#include "soundresource.hpp"
#include "soundbuffer.hpp"

namespace cursedearth
{
    struct ce_sound_instance
    {
        ce_sound_object sound_object;
        ce_sound_resource* sound_resource;
        ce_sound_buffer* sound_buffer;
        std::atomic<int> state;
        std::atomic<float> time;
        std::atomic<bool> done;
        ce_thread* thread;
    };

    ce_sound_instance* ce_sound_instance_new(ce_sound_object, ce_sound_resource*);
    void ce_sound_instance_del(ce_sound_instance*);

    void ce_sound_instance_advance(ce_sound_instance*, float elapsed);
    void ce_sound_instance_change_state(ce_sound_instance*, int state);
}

#endif
