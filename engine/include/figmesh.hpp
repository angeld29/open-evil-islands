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

#ifndef CE_FIGMESH_HPP
#define CE_FIGMESH_HPP

#include "vector.hpp"
#include "atomic.hpp"
#include "figproto.hpp"
#include "complection.hpp"
#include "renderitem.hpp"

typedef struct {
    int ref_count;
    ce_figproto* figproto;
    ce_complection complection;
    ce_vector* renderitems;
} ce_figmesh;

extern ce_figmesh* ce_figmesh_new(ce_figproto* figproto, const ce_complection* complection);
extern void ce_figmesh_del(ce_figmesh* figmesh);

static inline ce_figmesh* ce_figmesh_add_ref(ce_figmesh* figmesh)
{
    ce_atomic_inc(int, &figmesh->ref_count);
    return figmesh;
}

#endif /* CE_FIGMESH_HPP */
