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

#include <string.h>
#include <assert.h>

#include <GL/gl.h>

#include "cemath.h"
#include "celogging.h"
#include "cealloc.h"
#include "cefighlp.h"
#include "ceanmstate.h"
#include "cefigrenderitem.h"

static void
ce_figrenderitem_base_init(ce_renderitem* renderitem,
							const ce_figfile* figfile,
							const ce_complection* complection)
{
	ce_fighlp_get_aabb(&renderitem->bounding_box, figfile, complection);
	ce_fighlp_get_sphere(&renderitem->bounding_sphere, figfile, complection);

	renderitem->transparent = false;
}

typedef struct {
	GLuint id;
} ce_figrenderitem_static;

static bool
ce_figrenderitem_static_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_figrenderitem_static* figrenderitem =
		(ce_figrenderitem_static*)renderitem->impl;

	const ce_figfile* figfile = va_arg(args, const ce_figfile*);
	const ce_complection* complection = va_arg(args, const ce_complection*);

	ce_figrenderitem_base_init(renderitem, figfile, complection);

	if (0 == (figrenderitem->id = glGenLists(1))) {
		ce_logging_error("figrenderitem: could not generate display list");
		return false;
	}

	glNewList(figrenderitem->id, GL_COMPILE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// TODO: to be unhardcoded...
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float array[3];

	glBegin(GL_TRIANGLES);
	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		int index = figfile->indices[i];

		int vertex_index = figfile->spec_components[3 * index + 0];
		int normal_index = figfile->spec_components[3 * index + 1];
		int texcoord_index = figfile->spec_components[3 * index + 2];

		glTexCoord2fv(figfile->texcoords + 2 * texcoord_index);
		glNormal3fv(ce_fighlp_get_normal(array, figfile, normal_index));
		glVertex3fv(ce_fighlp_get_vertex(
			array, figfile, vertex_index, complection));
	}
	glEnd();

	glDisable(GL_BLEND);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEndList();

	return true;
}

static void ce_figrenderitem_static_dtor(ce_renderitem* renderitem)
{
	ce_figrenderitem_static* figrenderitem =
		(ce_figrenderitem_static*)renderitem->impl;

	glDeleteLists(figrenderitem->id, 1);
}

static void ce_figrenderitem_static_render(ce_renderitem* renderitem)
{
	ce_figrenderitem_static* figrenderitem =
		(ce_figrenderitem_static*)renderitem->impl;

	glCallList(figrenderitem->id);
}

typedef struct {
	int vertex_count;
	float* initial_vertices;
	float* morphed_vertices;
	float* normals;
	float* texcoords;
} ce_figrenderitem_dynamic;

static void
ce_figrenderitem_dynamic_clean(ce_figrenderitem_dynamic* figrenderitem)
{
	ce_free(figrenderitem->texcoords,
			sizeof(float) * 2 * figrenderitem->vertex_count);
	ce_free(figrenderitem->normals,
			sizeof(float) * 3 * figrenderitem->vertex_count);
	ce_free(figrenderitem->morphed_vertices,
			sizeof(float) * 3 * figrenderitem->vertex_count);
	ce_free(figrenderitem->initial_vertices,
			sizeof(float) * 3 * figrenderitem->vertex_count);
}

static bool
ce_figrenderitem_dynamic_init(ce_figrenderitem_dynamic* figrenderitem,
								int vertex_count)
{
	figrenderitem->vertex_count = vertex_count;

	if (NULL == (figrenderitem->initial_vertices =
				ce_alloc(sizeof(float) * 3 * vertex_count)) ||
			NULL == (figrenderitem->morphed_vertices =
					ce_alloc(sizeof(float) * 3 * vertex_count)) ||
			NULL == (figrenderitem->normals =
					ce_alloc(sizeof(float) * 3 * vertex_count)) ||
			NULL == (figrenderitem->texcoords =
					ce_alloc(sizeof(float) * 2 * vertex_count))) {
		ce_logging_error("figrenderitem: could not allocate memory");
		ce_figrenderitem_dynamic_clean(figrenderitem);
		return false;
	}

	return true;
}

static bool
ce_figrenderitem_dynamic_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	const ce_figfile* figfile = va_arg(args, const ce_figfile*);
	const ce_complection* complection = va_arg(args, const ce_complection*);

	ce_figrenderitem_base_init(renderitem, figfile, complection);

	if (!ce_figrenderitem_dynamic_init(figrenderitem, figfile->index_count)) {
		return false;
	}

	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		int index = figfile->indices[i];
		int vertex_index = figfile->spec_components[3 * index + 0];
		int normal_index = figfile->spec_components[3 * index + 1];
		int texcoord_index = figfile->spec_components[3 * index + 2];

		ce_fighlp_get_vertex(figrenderitem->initial_vertices + 3 * i,
							figfile, vertex_index, complection);

		ce_fighlp_get_normal(figrenderitem->normals + 3 * i,
							figfile, normal_index);

		figrenderitem->texcoords[2 * i + 0] =
			figfile->texcoords[2 * texcoord_index + 0];
		figrenderitem->texcoords[2 * i + 1] =
			figfile->texcoords[2 * texcoord_index + 1];
	}

	memcpy(figrenderitem->morphed_vertices,
			figrenderitem->initial_vertices,
			sizeof(float) * 3 * figrenderitem->vertex_count);

	return true;
}

static void ce_figrenderitem_dynamic_dtor(ce_renderitem* renderitem)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	ce_figrenderitem_dynamic_clean(figrenderitem);
}

static void
ce_figrenderitem_dynamic_update(ce_renderitem* renderitem, va_list args)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	const ce_figfile* figfile = va_arg(args, const ce_figfile*);
	const ce_anmstate* anmstate = va_arg(args, const ce_anmstate*);

	const float* prev_morphs = anmstate->anmfile->morphs +
								(int)anmstate->prev_frame * 3 *
									anmstate->anmfile->morph_vertex_count;
	const float* next_morphs = anmstate->anmfile->morphs +
								(int)anmstate->next_frame * 3 *
									anmstate->anmfile->morph_vertex_count;

	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		int index = figfile->indices[i];
		int vertex_index = figfile->spec_components[3 * index];
		int morph_index = figfile->morph_components[2 * vertex_index];

		for (int j = 0; j < 3; ++j) {
			figrenderitem->morphed_vertices[3 * i + j] =
				figrenderitem->initial_vertices[3 * i + j] +
					ce_lerp(anmstate->coef, prev_morphs[3 * morph_index + j],
											next_morphs[3 * morph_index + j]);
		}
	}
}

static void ce_figrenderitem_dynamic_render(ce_renderitem* renderitem)
{
	ce_figrenderitem_dynamic* figrenderitem =
		(ce_figrenderitem_dynamic*)renderitem->impl;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, figrenderitem->morphed_vertices);
	glNormalPointer(GL_FLOAT, 0, figrenderitem->normals);
	glTexCoordPointer(2, GL_FLOAT, 0, figrenderitem->texcoords);

	// TODO: to be unhardcoded...
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, figrenderitem->vertex_count);

	glDisable(GL_BLEND);

	glPopClientAttrib();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

static bool ce_figrenderitem_dynamic_clone(const ce_renderitem* renderitem,
											ce_renderitem* clone_renderitem)
{
	const ce_figrenderitem_dynamic* figrenderitem =
		(const ce_figrenderitem_dynamic*)renderitem->impl;

	ce_figrenderitem_dynamic* clone_figrenderitem =
		(ce_figrenderitem_dynamic*)clone_renderitem->impl;

	if (!ce_figrenderitem_dynamic_init(clone_figrenderitem,
										figrenderitem->vertex_count)) {
		return false;
	}

	memcpy(clone_figrenderitem->initial_vertices,
			figrenderitem->initial_vertices,
			sizeof(float) * 3 * figrenderitem->vertex_count);
	memcpy(clone_figrenderitem->morphed_vertices,
			figrenderitem->morphed_vertices,
			sizeof(float) * 3 * figrenderitem->vertex_count);
	memcpy(clone_figrenderitem->normals, figrenderitem->normals,
			sizeof(float) * 3 * figrenderitem->vertex_count);
	memcpy(clone_figrenderitem->texcoords, figrenderitem->texcoords,
			sizeof(float) * 2 * figrenderitem->vertex_count);

	return true;
}

static const ce_renderitem_vtable ce_figrenderitem_vtables[] = {
	{ ce_figrenderitem_static_ctor, ce_figrenderitem_static_dtor,
		NULL, ce_figrenderitem_static_render, NULL },
	{ ce_figrenderitem_dynamic_ctor, ce_figrenderitem_dynamic_dtor,
		ce_figrenderitem_dynamic_update, ce_figrenderitem_dynamic_render,
		ce_figrenderitem_dynamic_clone }
};

static const size_t ce_figrenderitem_sizes[] = {
	sizeof(ce_figrenderitem_static),
	sizeof(ce_figrenderitem_dynamic)
};

ce_renderitem* ce_figrenderitem_new(const ce_figfile* figfile,
									const ce_complection* complection,
									bool has_morphing)
{
	return ce_renderitem_new(ce_figrenderitem_vtables[has_morphing],
			ce_figrenderitem_sizes[has_morphing], figfile, complection);
}
