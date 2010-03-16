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

#ifndef CE_CFGFILE_H
#define CE_CFGFILE_H

#include "cestring.h"
#include "cevector.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_string* name;
	ce_string* value;
} ce_cfgfile_option;

typedef struct {
	ce_string* name;
	ce_vector* options;
} ce_cfgfile_section;

typedef struct {
	ce_vector* sections;
} ce_cfgfile;

extern ce_cfgfile* ce_cfgfile_open(const char* path);
extern void ce_cfgfile_close(ce_cfgfile* cfg);

extern int ce_cfgfile_section_index(ce_cfgfile* cfg, const char* section_name);
extern int ce_cfgfile_option_index(ce_cfgfile* cfg, int section_index,
													const char* option_name);
extern const char* ce_cfgfile_get(ce_cfgfile* cfg, int section_index,
													int option_index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CFGFILE_H */
