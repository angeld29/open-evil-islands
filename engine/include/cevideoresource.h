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

#ifndef CE_VIDEORESOURCE_H
#define CE_VIDEORESOURCE_H

#include <stddef.h>
#include <stdbool.h>

#include "cememfile.h"
#include "ceycbcr.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_videoresource ce_videoresource;

typedef struct {
	size_t (*size_hint)(ce_memfile* memfile);
	bool (*test)(ce_memfile* memfile);
	bool (*ctor)(ce_videoresource* videoresource);
	void (*dtor)(ce_videoresource* videoresource);
	bool (*read)(ce_videoresource* videoresource);
	bool (*reset)(ce_videoresource* videoresource);
} ce_videoresource_vtable;

struct ce_videoresource {
	unsigned int width, height;
	float fps, time;
	size_t frame_index, frame_count;
	ce_ycbcr ycbcr;
	ce_memfile* memfile;
	ce_videoresource_vtable vtable;
	size_t size;
	char impl[];
};

extern ce_videoresource* ce_videoresource_new(ce_memfile* memfile);
extern void ce_videoresource_del(ce_videoresource* videoresource);

static inline bool ce_videoresource_read(ce_videoresource* videoresource)
{
	return (*videoresource->vtable.read)(videoresource);
}

extern bool ce_videoresource_reset(ce_videoresource* videoresource);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VIDEORESOURCE_H */
