/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2011 Anton Kurkin, Yanis Kurganov.
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

#ifndef CE_TESSBLACKLIST_H
#define CE_TESSBLACKLIST_H

#include <stdbool.h>

#include "cevector.h"
#include "cestring.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ce_string* name;
	bool isNameEnd;
	bool isLeafNode;
	bool isAllExcept;
	ce_vector* child_sections;
} ce_tess_blacklist_section;

typedef struct {
	ce_vector* sections;
} ce_tess_blacklist;

extern ce_tess_blacklist* ce_blacklist;

extern ce_tess_blacklist* ce_blacklist_open(const char* path);
extern void ce_blacklist_close(ce_tess_blacklist* blacklist);

#ifdef __cplusplus
}
#endif

#endif /* CE_FIGUREMANAGER_H */
