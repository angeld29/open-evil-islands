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

#ifndef CE_TEXMNG_H
#define CE_TEXMNG_H

#include <stdbool.h>

#include "cevector.h"
#include "cetexture.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vector* resfiles;
	ce_vector* textures;
} ce_texmng;

extern ce_texmng* ce_texmng_new(void);
extern void ce_texmng_del(ce_texmng* texmng);

extern bool ce_texmng_register_resource(ce_texmng* texmng, const char* path);

extern ce_texture* ce_texmng_get_texture(ce_texmng* texmng, const char* name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TEXMNG_H */
