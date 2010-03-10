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

#include "celogging.h"
#include "cealloc.h"
#include "celnkfile.h"
#include "cefigproto.h"

static ce_lnkfile* ce_figproto_open_lnkfile(const char* name,
											ce_resfile* resfile)
{
	ce_memfile* memfile = ce_resfile_node_memfile_by_name(resfile, name);
	if (NULL == memfile) {
		ce_logging_error("figproto: could not open lnkfile: '%s'", name);
		return NULL;
	}

	ce_lnkfile* lnkfile = ce_lnkfile_open_memfile(memfile);
	return ce_memfile_close(memfile), lnkfile;
}

static ce_figfile* ce_figproto_open_figfile(const char* name,
											ce_resfile* resfile)
{
	ce_memfile* memfile = ce_resfile_node_memfile_by_name(resfile, name);
	if (NULL == memfile) {
		ce_logging_error("figproto: could not open figfile: '%s'", name);
		return NULL;
	}

	ce_figfile* figfile = ce_figfile_open_memfile(memfile);
	return ce_memfile_close(memfile), figfile;
}

static ce_bonfile* ce_figproto_open_bonfile(int value_count,
											const char* name,
											ce_resfile* resfile)
{
	ce_memfile* memfile = ce_resfile_node_memfile_by_name(resfile, name);
	if (NULL == memfile) {
		ce_logging_error("figproto: could not open bonfile: '%s'", name);
		return NULL;
	}

	ce_bonfile* bonfile = ce_bonfile_open_memfile(value_count, memfile);
	return ce_memfile_close(memfile), bonfile;
}

static ce_anmfile* ce_figproto_open_anmfile(const char* anim_name,
											const char* name,
											ce_resfile* resfile)
{
	ce_memfile* memfile = ce_resfile_node_memfile_by_name(resfile, name);
	if (NULL == memfile) {
		ce_logging_error("figproto: could not open anmfile: '%s'", name);
		return NULL;
	}

	ce_anmfile* anmfile = ce_anmfile_open_memfile(anim_name, memfile);
	return ce_memfile_close(memfile), anmfile;
}

static void ce_figproto_node_del(ce_figproto_node* node)
{
	if (NULL != node) {
		if (NULL != node->child_nodes) {
			for (int i = 0, n = ce_vector_count(node->child_nodes); i < n; ++i) {
				ce_figproto_node_del(ce_vector_at(node->child_nodes, i));
			}
			ce_vector_del(node->child_nodes);
		}
		if (NULL != node->anmfiles) {
			for (int i = 0, n = ce_vector_count(node->anmfiles); i < n; ++i) {
				ce_anmfile_close(ce_vector_at(node->anmfiles, i));
			}
			ce_vector_del(node->anmfiles);
		}
		ce_bonfile_close(node->bonfile);
		ce_figfile_close(node->figfile);
		ce_string_del(node->name);
		ce_free(node, sizeof(ce_figproto_node));
	}
}

static bool ce_figproto_node_new_impl(ce_figproto_node* node,
							ce_string* name, ce_resfile* mod_resfile,
							ce_resfile* bon_resfile, ce_vector* anm_resfiles)
{
	if (NULL == (node->name = ce_string_dup(name)) ||
			NULL == (node->anmfiles = ce_vector_new()) ||
			NULL == (node->child_nodes = ce_vector_new())) {
		return false;
	}

	if (NULL == (node->figfile =
				ce_figproto_open_figfile(ce_string_cstr(name), mod_resfile)) ||
			NULL == (node->bonfile =
				ce_figproto_open_bonfile(node->figfile->value_count,
										ce_string_cstr(name), bon_resfile))) {
		return false;
	}

	node->has_morphing = false;

	for (int i = 0, n = ce_vector_count(anm_resfiles); i < n; ++i) {
		ce_resfile* anm_resfile = ce_vector_at(anm_resfiles, i);
		if (-1 == ce_resfile_node_index(anm_resfile, ce_string_cstr(name))) {
			// there is no animation for this part
			continue;
		}

		ce_anmfile* anmfile =
			ce_figproto_open_anmfile(ce_resfile_name(anm_resfile),
									ce_string_cstr(name), anm_resfile);
		if (NULL == anmfile) {
			return false;
		}

		node->has_morphing = node->has_morphing || NULL != anmfile->morphs;
		ce_vector_push_back(node->anmfiles, anmfile);
	}

	return true;
}

static ce_figproto_node*
ce_figproto_node_new(ce_string* name, ce_resfile* mod_resfile,
					ce_resfile* bon_resfile, ce_vector* anm_resfiles)
{
	ce_figproto_node* node = ce_alloc_zero(sizeof(ce_figproto_node));
	if (NULL == node) {
		ce_logging_error("figproto: could not allocate memory");
		return NULL;
	}

	if (!ce_figproto_node_new_impl(node, name, mod_resfile,
									bon_resfile, anm_resfiles)) {
		ce_figproto_node_del(node);
		return NULL;
	}

	return node;
}

static void ce_figproto_del_resfiles(ce_vector* resfiles)
{
	for (int i = 0, n = ce_vector_count(resfiles); i < n; ++i) {
		ce_resfile_close(ce_vector_at(resfiles, i));
	}
	ce_vector_del(resfiles);
}

static ce_vector* ce_figproto_extract_resfiles(ce_resfile* resfile)
{
	ce_vector* child_resfiles =
		ce_vector_new_reserved(ce_resfile_node_count(resfile));
	if (NULL == child_resfiles) {
		return NULL;
	}

	for (int i = 0, n = ce_resfile_node_count(resfile); i < n; ++i) {
		ce_resfile* child_resfile = ce_resfile_node_resfile(resfile, i);
		if (NULL == child_resfile) {
			ce_figproto_del_resfiles(child_resfiles);
			return NULL;
		}
		ce_vector_push_back(child_resfiles, child_resfile);
	}

	return child_resfiles;
}

static bool
ce_figproto_create_nodes(ce_figproto* figproto, ce_figproto_node* parent_node,
						ce_resfile* mod_resfile, ce_resfile* bon_resfile,
						ce_vector* anm_resfiles, ce_lnkfile* lnkfile,
						unsigned int* index)
{
	while (*index < lnkfile->relationship_count &&
			(NULL == parent_node || 0 == ce_string_cmp(parent_node->name,
								lnkfile->relationships[*index].parent_name))) {
		ce_figproto_node* child_node =
			ce_figproto_node_new(lnkfile->relationships[*index].child_name,
								mod_resfile, bon_resfile, anm_resfiles);
		if (NULL == child_node) {
			return false;
		}

		if (NULL == parent_node) {
			assert(ce_string_empty(lnkfile->relationships[*index].parent_name));
			figproto->root_node = parent_node = child_node;
		} else {
			ce_vector_push_back(parent_node->child_nodes, child_node);
		}

		++*index;

		if (!ce_figproto_create_nodes(figproto, parent_node, mod_resfile,
					bon_resfile, anm_resfiles, lnkfile, index) ||
				!ce_figproto_create_nodes(figproto, child_node, mod_resfile,
					bon_resfile, anm_resfiles, lnkfile, index)) {
			return false;
		}
	}

	return true;
}

static bool ce_figproto_new_tri_resfile(ce_figproto* figproto,
							const char* name, ce_resfile* mod_resfile,
							ce_resfile* bon_resfile, ce_vector* anm_resfiles)
{
	ce_lnkfile* lnkfile = ce_figproto_open_lnkfile(name, mod_resfile);
	if (NULL == lnkfile) {
		ce_logging_error("figproto: could not open lnkfile");
		return false;
	}

	unsigned int index = 0;
	bool ok = ce_figproto_create_nodes(figproto, NULL,
											mod_resfile, bon_resfile,
											anm_resfiles, lnkfile, &index);
	return ce_lnkfile_close(lnkfile), ok;
}

static bool ce_figproto_new_impl(ce_figproto* figproto,
								const char* figure_name, ce_resfile* resfile)
{
	figproto->ref_count = 1;

	if (NULL == (figproto->name = ce_string_new_cstr(figure_name))) {
		return false;
	}

	char file_name[strlen(figure_name) + 4 + 1];

	snprintf(file_name, sizeof(file_name), "%s.mod", figure_name);
	ce_resfile* mod_resfile =
		ce_resfile_node_resfile_by_name(resfile, file_name);
	if (NULL == mod_resfile) {
		return false;
	}

	snprintf(file_name, sizeof(file_name), "%s.bon", figure_name);
	ce_resfile* bon_resfile =
		ce_resfile_node_resfile_by_name(resfile, file_name);
	assert(NULL != bon_resfile);

	snprintf(file_name, sizeof(file_name), "%s.anm", figure_name);
	ce_resfile* anm_resfile =
		ce_resfile_node_resfile_by_name(resfile, file_name);

	ce_vector* anm_resfiles = NULL == anm_resfile ? ce_vector_new_reserved(0) :
									ce_figproto_extract_resfiles(anm_resfile);
	if (NULL == anm_resfiles) {
		ce_resfile_close(anm_resfile);
		ce_resfile_close(bon_resfile);
		ce_resfile_close(mod_resfile);
		return false;
	}

	bool ok = ce_figproto_new_tri_resfile(figproto, figure_name, mod_resfile,
											bon_resfile, anm_resfiles);

	ce_figproto_del_resfiles(anm_resfiles);

	ce_resfile_close(anm_resfile);
	ce_resfile_close(bon_resfile);
	ce_resfile_close(mod_resfile);

	return ok;
}

ce_figproto* ce_figproto_new(const char* figure_name, ce_resfile* resfile)
{
	ce_figproto* figproto = ce_alloc_zero(sizeof(ce_figproto));
	if (NULL == figproto) {
		ce_logging_error("figproto: could not allocate memory");
		return NULL;
	}

	if (!ce_figproto_new_impl(figproto, figure_name, resfile)) {
		ce_figproto_del(figproto);
		return NULL;
	}

	return figproto;
}

void ce_figproto_del(ce_figproto* figproto)
{
	if (NULL != figproto) {
		assert(figproto->ref_count > 0);
		if (0 == --figproto->ref_count) {
			ce_figproto_node_del(figproto->root_node);
			ce_string_del(figproto->name);
			ce_free(figproto, sizeof(ce_figproto));
		}
	}
}

ce_figproto* ce_figproto_copy(ce_figproto* figproto)
{
	++figproto->ref_count;
	return figproto;
}
