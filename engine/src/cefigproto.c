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
#include "cereshlp.h"
#include "cefigproto.h"

ce_figproto* ce_figproto_new(const char* name, ce_resfile* resfile)
{
	ce_figproto* figproto = ce_alloc(sizeof(ce_figproto));
	figproto->name = ce_string_new_str(name);
	figproto->ref_count = 1;

	char file_name[strlen(name) + 4 + 1];

	snprintf(file_name, sizeof(file_name), "%s.mod", name);
	ce_resfile* mod_resfile =
		ce_reshlp_extract_resfile_by_name(resfile, file_name);

	snprintf(file_name, sizeof(file_name), "%s.bon", name);
	ce_resfile* bon_resfile =
		ce_reshlp_extract_resfile_by_name(resfile, file_name);

	snprintf(file_name, sizeof(file_name), "%s.anm", name);
	ce_resfile* anm_resfile =
		ce_reshlp_extract_resfile_by_name(resfile, file_name);

	assert(NULL != mod_resfile); // mod required
	assert(NULL != bon_resfile); // bon required

	// anm optional
	ce_vector* anm_resfiles = NULL == anm_resfile ?
								ce_vector_new_reserved(0) :
								ce_reshlp_extract_all_resfiles(anm_resfile);

	ce_lnkfile* lnkfile = ce_lnkfile_open(mod_resfile, name);

	figproto->fignode = ce_fignode_new(mod_resfile, bon_resfile,
										anm_resfiles, lnkfile);

	ce_lnkfile_close(lnkfile);
	ce_reshlp_del_resfiles(anm_resfiles);
	ce_resfile_close(anm_resfile);
	ce_resfile_close(bon_resfile);
	ce_resfile_close(mod_resfile);

	return figproto;
}

void ce_figproto_del(ce_figproto* figproto)
{
	if (NULL != figproto) {
		assert(figproto->ref_count > 0);
		if (0 == --figproto->ref_count) {
			ce_fignode_del(figproto->fignode);
			ce_string_del(figproto->name);
			ce_free(figproto, sizeof(ce_figproto));
		}
	}
}

ce_figproto* ce_figproto_add_ref(ce_figproto* figproto)
{
	++figproto->ref_count;
	return figproto;
}

void ce_figproto_accept_renderqueue(ce_figproto* figproto,
									ce_renderqueue* renderqueue)
{
	ce_fignode_accept_renderqueue_cascade(figproto->fignode, renderqueue);
}
