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

#ifndef CE_MPRFILE_H
#define CE_MPRFILE_H

#include <stdbool.h>

#include "ceresfilefwd.h"
#include "cefrustum.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_mprfile ce_mprfile;

extern ce_mprfile* ce_mprfile_open(ce_resfile* mpr_res, ce_resfile* textures_res);
extern void ce_mprfile_close(ce_mprfile* mpr);

extern float ce_mprfile_get_max_height(const ce_mprfile* mpr);

extern void ce_mprfile_apply_frustum(ce_mprfile* mpr, const vec3* eye,
														const ce_frustum* f);

extern void ce_mprfile_render(ce_mprfile* mpr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MPRFILE_H */
