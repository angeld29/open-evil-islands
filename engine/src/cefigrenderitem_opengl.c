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
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "cegl.h"
#include "cemath.h"
#include "cealloc.h"
#include "cefighlp.h"
#include "ceanmstate.h"
#include "cefigrenderitem.h"

// fig renderitem static (without morphs): gl's display list

typedef struct {
	GLuint id;
	int ref_count;
} ce_figcookie_static;

static ce_figcookie_static* ce_figcookie_static_new(void)
{
	ce_figcookie_static* cookie = ce_alloc(sizeof(ce_figcookie_static));
	cookie->id = glGenLists(1);
	cookie->ref_count = 1;
	return cookie;
}

static void ce_figcookie_static_del(ce_figcookie_static* cookie)
{
	if (NULL != cookie) {
		assert(cookie->ref_count > 0);
		if (0 == --cookie->ref_count) {
			glDeleteLists(cookie->id, 1);
			ce_free(cookie, sizeof(ce_figcookie_static));
		}
	}
}

static ce_figcookie_static*
ce_figcookie_static_add_ref(ce_figcookie_static* cookie)
{
	++cookie->ref_count;
	return cookie;
}

typedef struct {
	ce_figcookie_static* cookie;
} ce_figrenderitem_static;

static void
ce_figrenderitem_static_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_figrenderitem_static* figrenderitem =
		(ce_figrenderitem_static*)renderitem->impl;

	const ce_figfile* figfile = va_arg(args, const ce_figfile*);
	const ce_complection* complection = va_arg(args, const ce_complection*);

	figrenderitem->cookie = ce_figcookie_static_new();

	glNewList(figrenderitem->cookie->id, GL_COMPILE);

	float array[3];

	glBegin(GL_TRIANGLES);
	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		int index = figfile->indices[i];
		int vertex_index = figfile->vertex_components[3 * index + 0];
		int normal_index = figfile->vertex_components[3 * index + 1];
		int texcoord_index = figfile->vertex_components[3 * index + 2];

		glTexCoord2fv(figfile->texcoords + 2 * texcoord_index);
		glNormal3fv(ce_fighlp_get_normal(array, figfile, normal_index));
		glVertex3fv(ce_fighlp_get_vertex(array, figfile, vertex_index,
														complection));
	}
	glEnd();

	glEndList();
}

static void ce_figrenderitem_static_dtor(ce_renderitem* renderitem)
{
	ce_figrenderitem_static* figrenderitem =
		(ce_figrenderitem_static*)renderitem->impl;

	ce_figcookie_static_del(figrenderitem->cookie);
}

static void ce_figrenderitem_static_render(ce_renderitem* renderitem)
{
	ce_figrenderitem_static* figrenderitem =
		(ce_figrenderitem_static*)renderitem->impl;

	glCallList(figrenderitem->cookie->id);
}

static void ce_figrenderitem_static_clone(const ce_renderitem* renderitem,
											ce_renderitem* clone_renderitem)
{
	const ce_figrenderitem_static* figrenderitem =
		(const ce_figrenderitem_static*)renderitem->impl;
	ce_figrenderitem_static* clone_figrenderitem =
		(ce_figrenderitem_static*)clone_renderitem->impl;

	clone_figrenderitem->cookie =
		ce_figcookie_static_add_ref(figrenderitem->cookie);
}

// fig renderitem dynamic (with morphs): gl's vertex buffer object or vertex array

typedef struct {
	int vertex_count;
	int ref_count;
	float* vertices; // initial vertices
	union {
		float* pointer;
		GLuint buffer;
	} normals;
	union {
		float* pointer;
		GLuint buffer;
	} texcoords;
} ce_figcookie_dynamic;

static ce_figcookie_dynamic* ce_figcookie_dynamic_new(int vertex_count)
{
	ce_figcookie_dynamic* cookie = ce_alloc(sizeof(ce_figcookie_dynamic));
	cookie->vertex_count = vertex_count;
	cookie->ref_count = 1;
	cookie->vertices = ce_alloc(sizeof(float) * 3 * vertex_count);
	if (GLEW_VERSION_1_5) {
		glGenBuffers(1, &cookie->normals.buffer);
		glGenBuffers(1, &cookie->texcoords.buffer);
	} else {
		cookie->normals.pointer = ce_alloc(sizeof(float) * 3 * vertex_count);
		cookie->texcoords.pointer = ce_alloc(sizeof(float) * 2 * vertex_count);
	}
	return cookie;
}

static void ce_figcookie_dynamic_del(ce_figcookie_dynamic* cookie)
{
	if (NULL != cookie) {
		assert(cookie->ref_count > 0);
		if (0 == --cookie->ref_count) {
			if (GLEW_VERSION_1_5) {
				glDeleteBuffers(1, &cookie->texcoords.buffer);
				glDeleteBuffers(1, &cookie->normals.buffer);
			} else {
				ce_free(cookie->texcoords.pointer,
						sizeof(float) * 2 * cookie->vertex_count);
				ce_free(cookie->normals.pointer,
						sizeof(float) * 3 * cookie->vertex_count);
			}
			ce_free(cookie->vertices,
					sizeof(float) * 3 * cookie->vertex_count);
			ce_free(cookie, sizeof(ce_figcookie_dynamic));
		}
	}
}

static ce_figcookie_dynamic*
ce_figcookie_dynamic_add_ref(ce_figcookie_dynamic* cookie)
{
	++cookie->ref_count;
	return cookie;
}

typedef struct {
	ce_figcookie_dynamic* cookie;
	float* vertices; // copy of initial vertices to store a morphing result
} ce_figrenderitem_dynamic;

static void
ce_figrenderitem_dynamic_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	const ce_figfile* figfile = va_arg(args, const ce_figfile*);
	const ce_complection* complection = va_arg(args, const ce_complection*);

	figrenderitem->cookie = ce_figcookie_dynamic_new(figfile->index_count);
	figrenderitem->vertices = ce_alloc(sizeof(float) * 3 * figfile->index_count);

	float* normals;
	float* texcoords;

	if (GLEW_VERSION_1_5) {
		glBindBuffer(GL_ARRAY_BUFFER, figrenderitem->cookie->normals.buffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) *
			figfile->index_count, NULL, GL_STATIC_DRAW);
		normals = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

		glBindBuffer(GL_ARRAY_BUFFER, figrenderitem->cookie->texcoords.buffer);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) *
			figfile->index_count, NULL, GL_STATIC_DRAW);
		texcoords = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	} else {
		normals = figrenderitem->cookie->normals.pointer;
		texcoords = figrenderitem->cookie->texcoords.pointer;
	}

	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		int index = figfile->indices[i];
		int vertex_index = figfile->vertex_components[3 * index + 0];
		int normal_index = figfile->vertex_components[3 * index + 1];
		int texcoord_index = figfile->vertex_components[3 * index + 2];

		ce_fighlp_get_vertex(figrenderitem->cookie->vertices + 3 * i,
								figfile, vertex_index, complection);

		ce_fighlp_get_normal(normals + 3 * i, figfile, normal_index);

		texcoords[2 * i + 0] = figfile->texcoords[2 * texcoord_index + 0];
		texcoords[2 * i + 1] = figfile->texcoords[2 * texcoord_index + 1];
	}

	if (GLEW_VERSION_1_5) {
		glBindBuffer(GL_ARRAY_BUFFER, figrenderitem->cookie->normals.buffer);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, figrenderitem->cookie->texcoords.buffer);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	memcpy(figrenderitem->vertices,
			figrenderitem->cookie->vertices,
			sizeof(float) * 3 * figfile->index_count);
}

static void ce_figrenderitem_dynamic_dtor(ce_renderitem* renderitem)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	ce_free(figrenderitem->vertices,
			sizeof(float) * 3 * figrenderitem->cookie->vertex_count);
	ce_figcookie_dynamic_del(figrenderitem->cookie);
}

static void
ce_figrenderitem_dynamic_update(ce_renderitem* renderitem, va_list args)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	const ce_figfile* figfile = va_arg(args, const ce_figfile*);
	const ce_anmstate* anmstate = va_arg(args, const ce_anmstate*);

	if (NULL == anmstate->anmfile || NULL == anmstate->anmfile->morphs) {
		// initial state
		memcpy(figrenderitem->vertices,
				figrenderitem->cookie->vertices,
				sizeof(float) * 3 * figrenderitem->cookie->vertex_count);
		ce_aabb_clear(&renderitem->aabb);
		for (int i = 0; i < figrenderitem->cookie->vertex_count; ++i) {
			ce_aabb_merge_point_array(&renderitem->aabb,
				figrenderitem->vertices + 3 * i);
		}
		ce_aabb_update_radius(&renderitem->aabb);
		return;
	}

	const float* prev_morphs = anmstate->anmfile->morphs +
								(int)anmstate->prev_frame * 3 *
									anmstate->anmfile->morph_vertex_count;
	const float* next_morphs = anmstate->anmfile->morphs +
								(int)anmstate->next_frame * 3 *
									anmstate->anmfile->morph_vertex_count;

	ce_aabb_clear(&renderitem->aabb);

	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		int index = figfile->indices[i];
		int vertex_index = figfile->vertex_components[3 * index];
		int morph_index = figfile->morph_components[2 * vertex_index];

		for (int j = 0; j < 3; ++j) {
			figrenderitem->vertices[3 * i + j] =
				figrenderitem->cookie->vertices[3 * i + j] +
					ce_lerp(anmstate->coef, prev_morphs[3 * morph_index + j],
											next_morphs[3 * morph_index + j]);
		}

		ce_aabb_merge_point_array(&renderitem->aabb,
			figrenderitem->vertices + 3 * i);
	}

	ce_aabb_update_radius(&renderitem->aabb);
}

static void ce_figrenderitem_dynamic_render(ce_renderitem* renderitem)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, figrenderitem->vertices);

	if (GLEW_VERSION_1_5) {
		glBindBuffer(GL_ARRAY_BUFFER, figrenderitem->cookie->normals.buffer);
		glNormalPointer(GL_FLOAT, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, figrenderitem->cookie->texcoords.buffer);
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	} else {
		glNormalPointer(GL_FLOAT, 0, figrenderitem->cookie->normals.pointer);
		glTexCoordPointer(2, GL_FLOAT, 0, figrenderitem->cookie->texcoords.pointer);
	}

	glDrawArrays(GL_TRIANGLES, 0, figrenderitem->cookie->vertex_count);

	glPopClientAttrib();
}

static void ce_figrenderitem_dynamic_clone(const ce_renderitem* renderitem,
											ce_renderitem* clone_renderitem)
{
	const ce_figrenderitem_dynamic* figrenderitem =
		(const ce_figrenderitem_dynamic*)renderitem->impl;
	ce_figrenderitem_dynamic* clone_figrenderitem =
		(ce_figrenderitem_dynamic*)clone_renderitem->impl;

	clone_figrenderitem->cookie =
		ce_figcookie_dynamic_add_ref(figrenderitem->cookie);
	clone_figrenderitem->vertices =
		ce_alloc(sizeof(float) * 3 * figrenderitem->cookie->vertex_count);

	memcpy(clone_figrenderitem->vertices,
			figrenderitem->vertices,
			sizeof(float) * 3 * figrenderitem->cookie->vertex_count);
}

static const ce_renderitem_vtable ce_figrenderitem_vtables[] = {
	{ ce_figrenderitem_static_ctor, ce_figrenderitem_static_dtor,
		NULL, ce_figrenderitem_static_render, ce_figrenderitem_static_clone },
	{ ce_figrenderitem_dynamic_ctor, ce_figrenderitem_dynamic_dtor,
		ce_figrenderitem_dynamic_update, ce_figrenderitem_dynamic_render,
		ce_figrenderitem_dynamic_clone }
};

static const size_t ce_figrenderitem_sizes[] = {
	sizeof(ce_figrenderitem_static),
	sizeof(ce_figrenderitem_dynamic)
};

ce_renderitem* ce_figrenderitem_new(const ce_fignode* fignode,
									const ce_complection* complection)
{
	bool has_morphing = false;
	for (int i = 0; i < fignode->anmfiles->count; ++i) {
		ce_anmfile* anmfile = fignode->anmfiles->items[i];
		has_morphing = has_morphing || NULL != anmfile->morphs;
	}

	return ce_renderitem_new(ce_figrenderitem_vtables[has_morphing],
							ce_figrenderitem_sizes[has_morphing],
							fignode->figfile, complection);
}
