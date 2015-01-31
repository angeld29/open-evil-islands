/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

/*
 *  Platform-independent atomic operations
 */

#ifndef CE_ATOMIC_HPP
#define CE_ATOMIC_HPP

#include <stddef.h>

#define ce_atomic_fetch_and_add(T, ptr, val) ce_atomic_fetch_and_add_##T(ptr, val)
#define ce_atomic_fetch_and_sub(T, ptr, val) ce_atomic_fetch_and_sub_##T(ptr, val)

#define ce_atomic_add_and_fetch(T, ptr, val) ce_atomic_add_and_fetch_##T(ptr, val)
#define ce_atomic_sub_and_fetch(T, ptr, val) ce_atomic_sub_and_fetch_##T(ptr, val)

#define ce_atomic_fetch_and_inc(T, ptr) ce_atomic_fetch_and_add(T, ptr, 1)
#define ce_atomic_fetch_and_dec(T, ptr) ce_atomic_fetch_and_sub(T, ptr, 1)

#define ce_atomic_inc_and_fetch(T, ptr) ce_atomic_add_and_fetch(T, ptr, 1)
#define ce_atomic_dec_and_fetch(T, ptr) ce_atomic_sub_and_fetch(T, ptr, 1)

#define ce_atomic_add(T, ptr, val) ce_atomic_add_and_fetch(T, ptr, val)
#define ce_atomic_sub(T, ptr, val) ce_atomic_sub_and_fetch(T, ptr, val)

#define ce_atomic_inc(T, ptr) ce_atomic_inc_and_fetch(T, ptr)
#define ce_atomic_dec(T, ptr) ce_atomic_dec_and_fetch(T, ptr)

#define ce_atomic_fetch(T, ptr) ce_atomic_add_and_fetch(T, ptr, 0)

#define CE_ATOMIC_DECL_FETCH_AND_OP(T, op) \
extern T ce_atomic_fetch_and_##op##_##T(T* ptr, T val);

#define CE_ATOMIC_DECL_OP_AND_FETCH(T, op) \
extern T ce_atomic_##op##_and_fetch_##T(T* ptr, T val);

#define CE_ATOMIC_DECL_ALL(T) \
CE_ATOMIC_DECL_FETCH_AND_OP(T, add) \
CE_ATOMIC_DECL_FETCH_AND_OP(T, sub) \
CE_ATOMIC_DECL_OP_AND_FETCH(T, add) \
CE_ATOMIC_DECL_OP_AND_FETCH(T, sub)

CE_ATOMIC_DECL_ALL(int)
CE_ATOMIC_DECL_ALL(size_t)

#undef CE_ATOMIC_DECL_ALL
#undef CE_ATOMIC_DECL_OP_AND_FETCH
#undef CE_ATOMIC_DECL_FETCH_AND_OP

#endif /* CE_ATOMIC_HPP */
