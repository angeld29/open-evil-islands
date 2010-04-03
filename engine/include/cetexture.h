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

#ifndef CE_TEXTURE_H
#define CE_TEXTURE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_texture ce_texture;

extern ce_texture* ce_texture_new(const char* name, void* data);
extern void ce_texture_del(ce_texture* texture);

extern bool ce_texture_equal(const ce_texture* texture,
							const ce_texture* other);

extern const char* ce_texture_get_name(ce_texture* texture);

extern void ce_texture_bind(ce_texture* texture);
extern void ce_texture_unbind(ce_texture* texture);

extern ce_texture* ce_texture_add_ref(ce_texture* texture);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TEXTURE_H */
