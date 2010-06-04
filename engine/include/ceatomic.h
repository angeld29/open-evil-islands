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

#ifndef CE_ATOMIC_H
#define CE_ATOMIC_H

#include <stddef.h>

#define CE_ATOMIC_FETCH(type) \
static inline type ce_atomic_fetch_##type(type* ptr) \
{ \
	return ce_atomic_add_and_fetch_##type(ptr, 0); \
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 *  Platform-independent atomic operations.
*/

extern size_t ce_atomic_fetch_and_add_size_t(size_t* ptr, size_t value);
extern size_t ce_atomic_fetch_and_sub_size_t(size_t* ptr, size_t value);

extern size_t ce_atomic_add_and_fetch_size_t(size_t* ptr, size_t value);
extern size_t ce_atomic_sub_and_fetch_size_t(size_t* ptr, size_t value);

CE_ATOMIC_FETCH(size_t)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#undef CE_ATOMIC_FETCH

#endif /* CE_ATOMIC_H */
