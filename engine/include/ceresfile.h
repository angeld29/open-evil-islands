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

#ifndef CE_RESFILE_H
#define CE_RESFILE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "cestring.h"
#include "cememfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_string* name;
	int32_t next_index;
	uint32_t data_length;
	uint32_t data_offset;
	int32_t modified;
	uint16_t name_length;
	uint32_t name_offset;
} ce_resnode;

typedef struct {
	ce_string* name;
	uint32_t node_count;
	uint32_t metadata_offset;
	uint32_t names_length;
	char* names;
	ce_resnode* nodes;
	ce_memfile* mem;
} ce_resfile;

/// Resfile takes ownership of the memfile if successfull.
extern ce_resfile* ce_resfile_open_memfile(const char* name, ce_memfile* mem);
extern ce_resfile* ce_resfile_open_file(const char* path);
extern void ce_resfile_close(ce_resfile* res);

extern int ce_resfile_node_index(const ce_resfile* res, const char* name);

extern const char* ce_resfile_node_name(const ce_resfile* res, int index);
extern size_t ce_resfile_node_size(const ce_resfile* res, int index);
extern time_t ce_resfile_node_modified(const ce_resfile* res, int index);
extern void* ce_resfile_node_data(ce_resfile* res, int index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RESFILE_H */
