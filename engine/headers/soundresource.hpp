/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_SOUNDRESOURCE_HPP
#define CE_SOUNDRESOURCE_HPP

#include "memfile.hpp"
#include "soundbuffer.hpp"

namespace cursedearth
{
    enum {
        CE_SOUND_PROBE_BUFFER_CAPACITY = 1024
    };

    typedef struct {
        ce_mem_file* mem_file;
        const char* name;
        size_t impl_size;
        size_t input_buffer_capacity;
        size_t output_buffer_capacity;
        char buffer[CE_SOUND_PROBE_BUFFER_CAPACITY];
    } ce_sound_probe;

    typedef struct ce_sound_resource ce_sound_resource;

    typedef struct {
        bool (*test)(ce_sound_probe* sound_probe);
        bool (*ctor)(ce_sound_resource* sound_resource, ce_sound_probe* sound_probe);
        void (*dtor)(ce_sound_resource* sound_resource);
        bool (*decode)(ce_sound_resource* sound_resource);
        bool (*reset)(ce_sound_resource* sound_resource);
    } ce_sound_resource_vtable;

    struct ce_sound_resource {
        sound_format_t sound_format;
        size_t input_buffer_capacity;
        size_t output_buffer_capacity;
        size_t output_buffer_size;
        size_t output_buffer_pos;
        uint8_t* input_buffer;
        uint8_t* output_buffer;
        ce_mem_file* mem_file;
        ce_sound_resource_vtable vtable;
        size_t size;
        void* impl;
    };

    extern const size_t CE_SOUND_RESOURCE_BUILTIN_COUNT;
    extern const ce_sound_resource_vtable ce_sound_resource_builtins[];

    ce_sound_resource* ce_sound_resource_new(ce_mem_file*);
    void ce_sound_resource_del(ce_sound_resource*);

    bool ce_sound_resource_read(ce_sound_resource*, const sound_buffer_ptr_t&);
    bool ce_sound_resource_reset(ce_sound_resource*);
}

#endif
