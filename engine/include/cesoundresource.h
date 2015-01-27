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

#ifndef CE_SOUNDRESOURCE_H
#define CE_SOUNDRESOURCE_H

#include <stddef.h>
#include <stdbool.h>

#include "cememfile.h"
#include "cesoundformat.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    ce_sound_format sound_format;
    float time, bytes_per_second_inv;
    size_t granule_pos;
    size_t input_buffer_capacity;
    size_t output_buffer_capacity;
    size_t output_buffer_size;
    size_t output_buffer_pos;
    char* input_buffer;
    char* output_buffer;
    ce_mem_file* mem_file;
    ce_sound_resource_vtable vtable;
    size_t size;
    char impl[];
};

extern ce_sound_resource* ce_sound_resource_new(ce_mem_file* mem_file);
extern void ce_sound_resource_del(ce_sound_resource* sound_resource);

extern size_t ce_sound_resource_read(ce_sound_resource* sound_resource, void* data, size_t size);
extern bool ce_sound_resource_reset(ce_sound_resource* sound_resource);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDRESOURCE_H */
