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

#ifndef CE_ANMFILE_H
#define CE_ANMFILE_H

#include <stdint.h>

#include "cestring.h"
#include "cememfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_string* name;
	uint32_t rotation_anim_count;
	uint32_t translation_anim_count;
	uint32_t morph_anim_count;
	uint32_t morph_vertex_count;
	float* rotations;
	float* translations;
	float* morphs;
} ce_anmfile;

extern ce_anmfile* ce_anmfile_open(const char* name, ce_memfile* memfile);
extern void ce_anmfile_close(ce_anmfile* anmfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_ANMFILE_H */
