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

ce_mem_file* ce_reshlp_extract_mem_file(ce_res_file* res_file, size_t index)
{
	return ce_mem_file_new_data(ce_res_file_node_data(res_file, index),
								ce_res_file_node_size(res_file, index));
}

ce_mem_file* ce_reshlp_extract_mem_file_by_name(ce_res_file* res_file, const char* name)
{
	size_t index = ce_res_file_node_index(res_file, name);
	return res_file->node_count != index ? ce_reshlp_extract_mem_file(res_file, index) : NULL;
}

ce_vector* ce_reshlp_extract_all_mem_files(ce_res_file* res_file)
{
	ce_vector* mem_files = ce_vector_new_reserved(res_file->node_count);
	for (size_t i = 0; i < res_file->node_count; ++i) {
		ce_vector_push_back(mem_files, ce_reshlp_extract_mem_file(res_file, i));
		if (NULL == ce_vector_back(mem_files)) {
			ce_reshlp_del_mem_files(mem_files);
			return NULL;
		}
	}
	return mem_files;
}

void ce_reshlp_del_mem_files(ce_vector* mem_files)
{
	for (size_t i = 0; i < mem_files->count; ++i) {
		ce_mem_file_del(mem_files->items[i]);
	}
	ce_vector_del(mem_files);
}

ce_res_file* ce_reshlp_extract_res_file(ce_res_file* res_file, size_t index)
{
	ce_mem_file* mem_file = ce_reshlp_extract_mem_file(res_file, index);
	if (NULL == mem_file) {
		return NULL;
	}

	return ce_res_file_new(ce_res_file_node_name(res_file, index), mem_file);
}

ce_res_file* ce_reshlp_extract_res_file_by_name(ce_res_file* res_file, const char* name)
{
	size_t index = ce_res_file_node_index(res_file, name);
	return res_file->node_count != index ? ce_reshlp_extract_res_file(res_file, index) : NULL;
}

ce_vector* ce_reshlp_extract_all_res_files(ce_res_file* res_file)
{
	ce_vector* res_files = ce_vector_new_reserved(res_file->node_count);
	for (size_t i = 0; i < res_file->node_count; ++i) {
		ce_vector_push_back(res_files, ce_reshlp_extract_res_file(res_file, i));
		if (NULL == ce_vector_back(res_files)) {
			ce_reshlp_del_res_files(res_files);
			return NULL;
		}
	}
	return res_files;
}

void ce_reshlp_del_res_files(ce_vector* res_files)
{
	for (size_t i = 0; i < res_files->count; ++i) {
		ce_res_file_del(res_files->items[i]);
	}
	ce_vector_del(res_files);
}
