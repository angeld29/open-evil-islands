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

/*
 *  See also:
 *  1. doc/formats/camfile.txt
*/

#ifndef CE_CAMFILE_H
#define CE_CAMFILE_H

#include <stddef.h>
#include <stdint.h>

#include "cememfile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t time;
	uint32_t unknown;
	float position[3];
	float rotation[4];
} ce_cam_record;

typedef struct {
	size_t record_count;
	ce_cam_record records[];
} ce_cam_file;

extern ce_cam_file* ce_cam_file_new(ce_mem_file* mem_file);
extern void ce_cam_file_del(ce_cam_file* cam_file);

#ifdef __cplusplus
}
#endif

#endif /* CE_CAMFILE_H */
