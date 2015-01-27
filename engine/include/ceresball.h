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

#ifndef CE_RESBALL_H
#define CE_RESBALL_H

#include "cememfile.h"
#include "ceresfile.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline ce_mem_file* ce_res_ball_extract_mem_file(ce_res_file* res_file, size_t index)
{
    return ce_mem_file_new_data(ce_res_file_node_data(res_file, index),
                                ce_res_file_node_size(res_file, index));
}

static inline ce_mem_file* ce_res_ball_extract_mem_file_by_name(ce_res_file* res_file, const char* name)
{
    size_t index = ce_res_file_node_index(res_file, name);
    return res_file->node_count != index ? ce_res_ball_extract_mem_file(res_file, index) : NULL;
}

extern void ce_res_ball_extract_all_mem_files(ce_res_file* res_file, ce_mem_file* mem_files[]);
extern void ce_res_ball_clean_all_mem_files(ce_res_file* res_file, ce_mem_file* mem_files[]);

static inline ce_res_file* ce_res_ball_extract_res_file(ce_res_file* res_file, size_t index)
{
    return ce_res_file_new(ce_res_file_node_name(res_file, index),
                            ce_res_ball_extract_mem_file(res_file, index));
}

static inline ce_res_file* ce_res_ball_extract_res_file_by_name(ce_res_file* res_file, const char* name)
{
    size_t index = ce_res_file_node_index(res_file, name);
    return res_file->node_count != index ? ce_res_ball_extract_res_file(res_file, index) : NULL;
}

extern void ce_res_ball_extract_all_res_files(ce_res_file* res_file, ce_res_file* res_files[]);
extern void ce_res_ball_clean_all_res_files(ce_res_file* res_file, ce_res_file* res_files[]);

#ifdef __cplusplus
}
#endif

#endif /* CE_RESBALL_H */
