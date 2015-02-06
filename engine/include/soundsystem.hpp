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

#ifndef CE_SOUNDSYSTEM_HPP
#define CE_SOUNDSYSTEM_HPP

#include <atomic>

#include "thread.hpp"
#include "soundbuffer.hpp"

namespace cursedearth
{
    typedef struct {
        size_t size;
        bool (*ctor)(void);
        void (*dtor)(void);
        bool (*write)(const void* block);
    } ce_sound_system_vtable;

    extern struct ce_sound_system {
        ce_sound_format sound_format;
        unsigned int samples_per_second; // actual value supported by implementation/hardware
        ce_sound_buffer* sound_buffer;
        std::atomic<bool> done;
        ce_thread* thread;
        ce_sound_system_vtable vtable;
        void* impl;
    }* ce_sound_system;

    ce_sound_system_vtable ce_sound_system_platform(void);
    ce_sound_system_vtable ce_sound_system_null(void);

    void ce_sound_system_init(void);
    void ce_sound_system_term(void);

    void* ce_sound_system_map_block(void);
    void ce_sound_system_unmap_block(void);
}

#endif
