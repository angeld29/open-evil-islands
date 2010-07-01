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

#include "cealloc.h"
#include "cereshlp.h"

ce_mem_file* ce_reshlp_extract_mem_file(ce_resfile* resfile, int index)
{
	void* data = ce_resfile_node_data(resfile, index);
	if (NULL == data) {
		return NULL;
	}

	ce_mem_file* mem_file = ce_mem_file_new_data(data, ce_resfile_node_size(resfile, index));
	if (NULL == mem_file) {
		ce_free(data, ce_resfile_node_size(resfile, index));
		return NULL;
	}

	return mem_file;
}

ce_mem_file* ce_reshlp_extract_mem_file_by_name(ce_resfile* resfile,
												const char* name)
{
	int index = ce_resfile_node_index(resfile, name);
	return -1 != index ? ce_reshlp_extract_mem_file(resfile, index) : NULL;
}

ce_vector* ce_reshlp_extract_all_mem_files(ce_resfile* resfile)
{
	ce_vector* mem_files = ce_vector_new_reserved(resfile->node_count);
	for (int i = 0, n = resfile->node_count; i < n; ++i) {
		ce_vector_push_back(mem_files, ce_reshlp_extract_mem_file(resfile, i));
		if (NULL == ce_vector_back(mem_files)) {
			ce_reshlp_del_mem_files(mem_files);
			return NULL;
		}
	}
	return mem_files;
}

void ce_reshlp_del_mem_files(ce_vector* mem_files)
{
	for (int i = 0, n = mem_files->count; i < n; ++i) {
		ce_mem_file_del(mem_files->items[i]);
	}
	ce_vector_del(mem_files);
}

ce_resfile* ce_reshlp_extract_resfile(ce_resfile* resfile, int index)
{
	ce_mem_file* mem_file = ce_reshlp_extract_mem_file(resfile, index);
	if (NULL == mem_file) {
		return NULL;
	}

	ce_resfile* child_resfile = ce_resfile_open_mem_file(ce_resfile_node_name(resfile, index), mem_file);
	if (NULL == child_resfile) {
		ce_mem_file_del(mem_file);
		return NULL;
	}

	return child_resfile;
}

ce_resfile* ce_reshlp_extract_resfile_by_name(ce_resfile* resfile,
												const char* name)
{
	int index = ce_resfile_node_index(resfile, name);
	return -1 != index ? ce_reshlp_extract_resfile(resfile, index) : NULL;
}

ce_vector* ce_reshlp_extract_all_resfiles(ce_resfile* resfile)
{
	ce_vector* resfiles = ce_vector_new_reserved(resfile->node_count);
	for (int i = 0, n = resfile->node_count; i < n; ++i) {
		ce_vector_push_back(resfiles, ce_reshlp_extract_resfile(resfile, i));
		if (NULL == ce_vector_back(resfiles)) {
			ce_reshlp_del_resfiles(resfiles);
			return NULL;
		}
	}
	return resfiles;
}

void ce_reshlp_del_resfiles(ce_vector* resfiles)
{
	for (int i = 0, n = resfiles->count; i < n; ++i) {
		ce_resfile_close(resfiles->items[i]);
	}
	ce_vector_del(resfiles);
}
