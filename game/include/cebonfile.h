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

#ifndef CE_BONFILE_H
#define CE_BONFILE_H

#include "cememfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	int value_count;
	float* bone;
} ce_bonfile;

extern ce_bonfile* ce_bonfile_open(int value_count, ce_memfile* memfile);
extern void ce_bonfile_close(ce_bonfile* bonfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BONFILE_H */