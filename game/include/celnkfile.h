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

#ifndef CE_LNKFILE_H
#define CE_LNKFILE_H

#include "cestring.h"
#include "cememfile.h"
#include "ceresfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_string* child_name;
	ce_string* parent_name;
} ce_lnkfile_relationship;

typedef struct {
	int relationship_count;
	int relationship_index;
	ce_lnkfile_relationship* relationships;
} ce_lnkfile;

extern ce_lnkfile* ce_lnkfile_open_memfile(ce_memfile* memfile);
extern ce_lnkfile* ce_lnkfile_open_resfile(ce_resfile* resfile, const char* name);
extern void ce_lnkfile_close(ce_lnkfile* lnkfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LNKFILE_H */
