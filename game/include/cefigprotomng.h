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

#ifndef CE_FIGPROTOMNG_H
#define CE_FIGPROTOMNG_H

#include <stdbool.h>

#include "cevector.h"
#include "cefigproto.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vector* resources;
} ce_figprotomng;

extern ce_figprotomng* ce_figprotomng_new(void);
extern void ce_figprotomng_del(ce_figprotomng* figprotomng);

extern bool
ce_figprotomng_register_resource(ce_figprotomng* figprotomng, const char* path);

extern ce_figproto*
ce_figprotomng_get_figproto(ce_figprotomng* figprotomng, const char* name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGPROTOMNG_H */
