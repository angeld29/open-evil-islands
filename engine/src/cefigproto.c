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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cealloc.h"
#include "ceresourcemanager.h"
#include "ceresball.h"
#include "cefigproto.h"

ce_figproto* ce_figproto_new(const char* name, ce_res_file* res_file)
{
    ce_figproto* figproto = ce_alloc_zero(sizeof(ce_figproto));
    figproto->ref_count = 1;
    figproto->name = ce_string_new_str(name);

    char file_name[strlen(name) + 4 + 1];

    snprintf(file_name, sizeof(file_name), "%s.adb", name);
    ce_mem_file* adb_mem_file = ce_res_ball_extract_mem_file_by_name(ce_resource_manager->database, file_name);
    if (NULL != adb_mem_file) {
        figproto->adb_file = ce_adb_file_new(adb_mem_file);
        ce_mem_file_del(adb_mem_file);
    }

    snprintf(file_name, sizeof(file_name), "%s.mod", name);
    ce_res_file* mod_res_file = ce_res_ball_extract_res_file_by_name(res_file, file_name);

    snprintf(file_name, sizeof(file_name), "%s.bon", name);
    ce_res_file* bon_res_file = ce_res_ball_extract_res_file_by_name(res_file, file_name);

    snprintf(file_name, sizeof(file_name), "%s.anm", name);
    ce_res_file* anm_res_file = ce_res_ball_extract_res_file_by_name(res_file, file_name);

    assert(NULL != mod_res_file); // mod required
    assert(NULL != bon_res_file); // bon required

    // anm optional
    ce_res_file* anm_res_files[NULL == anm_res_file ? 1 : anm_res_file->node_count + 1];
    if (NULL != anm_res_file) {
        ce_res_ball_extract_all_res_files(anm_res_file, anm_res_files);
        anm_res_files[anm_res_file->node_count] = NULL;
    } else {
        anm_res_files[0] = NULL;
    }

    ce_lnkfile* lnkfile = ce_lnkfile_open(mod_res_file, name);

    figproto->fignode = ce_fignode_new(mod_res_file, bon_res_file,
                                        anm_res_files, lnkfile);

    ce_lnkfile_close(lnkfile);
    if (NULL != anm_res_file) {
        ce_res_ball_clean_all_res_files(anm_res_file, anm_res_files);
    }
    ce_res_file_del(anm_res_file);
    ce_res_file_del(bon_res_file);
    ce_res_file_del(mod_res_file);

    return figproto;
}

void ce_figproto_del(ce_figproto* figproto)
{
    if (NULL != figproto) {
        assert(ce_atomic_fetch(int, &figproto->ref_count) > 0);
        if (0 == ce_atomic_dec_and_fetch(int, &figproto->ref_count)) {
            ce_fignode_del(figproto->fignode);
            ce_adb_file_del(figproto->adb_file);
            ce_string_del(figproto->name);
            ce_free(figproto, sizeof(ce_figproto));
        }
    }
}

void ce_figproto_accept_renderqueue(ce_figproto* figproto,
                                    ce_renderqueue* renderqueue)
{
    ce_fignode_accept_renderqueue_cascade(figproto->fignode, renderqueue);
}
