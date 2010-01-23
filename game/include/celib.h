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

#ifndef CE_LIB_H
#define CE_LIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int cemin(int a, int b);
extern int cemax(int a, int b);
extern size_t cesmin(size_t a, size_t b);
extern size_t cesmax(size_t a, size_t b);

extern void ceswap(int* a, int* b);

extern int ceclamp(int v, int a, int b);
extern size_t cesclamp(size_t v, size_t a, size_t b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LIB_H */
