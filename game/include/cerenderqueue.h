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

#ifndef CE_RENDERQUEUE_H
#define CE_RENDERQUEUE_H

#include "cevec3.h"
#include "cevector.h"
#include "cefrustum.h"
#include "cerendersystem.h"
#include "cescenenode.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vector* scenenodes[2]; // opacity and transparent
} ce_renderqueue;

extern ce_renderqueue* ce_renderqueue_new(void);
extern void ce_renderqueue_del(ce_renderqueue* renderqueue);

extern void ce_renderqueue_clear(ce_renderqueue* renderqueue);

extern void ce_renderqueue_add(ce_renderqueue* renderqueue,
								ce_scenenode* scenenode,
								const ce_vec3* eye,
								const ce_frustum* frustum);
extern void ce_renderqueue_add_cascade(ce_renderqueue* renderqueue,
										ce_scenenode* scenenode,
										const ce_vec3* eye,
										const ce_frustum* frustum);

extern void ce_renderqueue_render(ce_renderqueue* renderqueue,
									ce_rendersystem* rendersystem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDERQUEUE_H */
