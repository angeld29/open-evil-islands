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
#include <time.h>

#include "memfilefwd.h"
#include "resfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/// Resfile takes ownership of the memfile if successfull.
extern resfile* resfile_open_memfile(const char* name, memfile* mem);
extern resfile* resfile_open_file(const char* path);
extern void resfile_close(resfile* res);

extern const char* resfile_name(const resfile* res);
extern int resfile_node_count(const resfile* res);
extern int resfile_node_index(const char* name, const resfile* res);

extern const char* resfile_node_name(int index, const resfile* res);
extern size_t resfile_node_size(int index, const resfile* res);
extern time_t resfile_node_modified(int index, const resfile* res);
extern bool resfile_node_data(int index, void* data, resfile* res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RESFILE_H */
