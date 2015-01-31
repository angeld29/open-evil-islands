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

#include "ceatomic.h"

#define CE_ATOMIC_DEF_FETCH_AND_OP(type, op) \
type ce_atomic_fetch_and_##op##_##type(type* ptr, type value) \
{ \
    return __extension__ __sync_fetch_and_##op(ptr, value); \
}

#define CE_ATOMIC_DEF_OP_AND_FETCH(type, op) \
type ce_atomic_##op##_and_fetch_##type(type* ptr, type value) \
{ \
    return __extension__ __sync_##op##_and_fetch(ptr, value); \
}

#define CE_ATOMIC_DEF_ALL(type) \
CE_ATOMIC_DEF_FETCH_AND_OP(type, add) \
CE_ATOMIC_DEF_FETCH_AND_OP(type, sub) \
CE_ATOMIC_DEF_OP_AND_FETCH(type, add) \
CE_ATOMIC_DEF_OP_AND_FETCH(type, sub)

CE_ATOMIC_DEF_ALL(int)
CE_ATOMIC_DEF_ALL(size_t)
