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

#ifndef CE_SOUNDRESOURCE_H
#define CE_SOUNDRESOURCE_H

#include <stddef.h>
#include <stdbool.h>

#include "cememfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_soundresource ce_soundresource;

typedef struct {
	size_t (*size_hint)(ce_memfile* memfile);
	bool (*test)(ce_memfile* memfile);
	bool (*ctor)(ce_soundresource* soundresource);
	void (*dtor)(ce_soundresource* soundresource);
	size_t (*read)(ce_soundresource* soundresource, void* data, size_t size);
	bool (*reset)(ce_soundresource* soundresource);
} ce_soundresource_vtable;

struct ce_soundresource {
	unsigned int bits_per_sample, sample_rate, channel_count;
	float time;
	ce_memfile* memfile;
	ce_soundresource_vtable vtable;
	size_t size;
	char impl[];
};

extern const size_t CE_SOUNDRESOURCE_BUILTIN_COUNT;
extern const ce_soundresource_vtable ce_soundresource_builtins[];

extern ce_soundresource* ce_soundresource_new(ce_soundresource_vtable vtable, ce_memfile* memfile);
extern ce_soundresource* ce_soundresource_new_builtin(ce_memfile* memfile);
extern void ce_soundresource_del(ce_soundresource* soundresource);

extern size_t ce_soundresource_find_builtin(ce_memfile* memfile);

static inline size_t ce_soundresource_read(ce_soundresource* soundresource, void* data, size_t size)
{
	return (*soundresource->vtable.read)(soundresource, data, size);
}

extern bool ce_soundresource_reset(ce_soundresource* soundresource);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SOUNDRESOURCE_H */
