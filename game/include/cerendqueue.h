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

#ifndef CE_RENDQUEUE_H
#define CE_RENDQUEUE_H

#include "cevec3.h"
#include "cevector.h"
#include "cefrustum.h"
#include "cerendlayer.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vector* opacity_renditems;
	ce_vector* transparent_renditems;
} ce_rendqueue;

extern ce_rendqueue* ce_rendqueue_new(void);
extern void ce_rendqueue_del(ce_rendqueue* rendqueue);

extern void ce_rendqueue_clear(ce_rendqueue* rendqueue);
extern void ce_rendqueue_render(ce_rendqueue* rendqueue);

extern void ce_rendqueue_add_rendlayer(ce_rendqueue* rendqueue,
										const ce_rendlayer* rendlayer,
										const ce_vec3* eye,
										const ce_frustum* frustum);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RENDQUEUE_H */
