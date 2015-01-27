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

#ifndef CE_COMPLECTION_H
#define CE_COMPLECTION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
    float dexterity, strength, height;
} ce_complection;

extern ce_complection*
ce_complection_init(ce_complection* complection, float dexterity,
                                                float strength, float height);
extern ce_complection*
ce_complection_init_array(ce_complection* complection, const float* array);

extern ce_complection*
ce_complection_copy(ce_complection* complection, const ce_complection* other);

extern bool ce_complection_equal(const ce_complection* complection,
                                const ce_complection* other);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_COMPLECTION_H */
