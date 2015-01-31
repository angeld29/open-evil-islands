/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "resball.hpp"

void ce_res_ball_extract_all_mem_files(ce_res_file* res_file, ce_mem_file* mem_files[])
{
    for (size_t i = 0; i < res_file->node_count; ++i) {
        mem_files[i] = ce_res_ball_extract_mem_file(res_file, i);
    }
}

void ce_res_ball_clean_all_mem_files(ce_res_file* res_file, ce_mem_file* mem_files[])
{
    for (size_t i = 0; i < res_file->node_count; ++i) {
        ce_mem_file_del(mem_files[i]);
    }
}

void ce_res_ball_extract_all_res_files(ce_res_file* res_file, ce_res_file* res_files[])
{
    for (size_t i = 0; i < res_file->node_count; ++i) {
        res_files[i] = ce_res_ball_extract_res_file(res_file, i);
    }
}

void ce_res_ball_clean_all_res_files(ce_res_file* res_file, ce_res_file* res_files[])
{
    for (size_t i = 0; i < res_file->node_count; ++i) {
        ce_res_file_del(res_files[i]);
    }
}
