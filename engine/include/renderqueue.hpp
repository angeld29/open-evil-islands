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

#ifndef CE_RENDERQUEUE_HPP
#define CE_RENDERQUEUE_HPP

#include "vector.hpp"
#include "rendergroup.hpp"

typedef struct {
    ce_vector* rendergroups;
} ce_renderqueue;

extern ce_renderqueue* ce_renderqueue_new(void);
extern void ce_renderqueue_del(ce_renderqueue* renderqueue);

extern void ce_renderqueue_clear(ce_renderqueue* renderqueue);

extern ce_rendergroup* ce_renderqueue_get(ce_renderqueue* renderqueue,
                                        int priority, ce_material* material);

extern void ce_renderqueue_render(ce_renderqueue* renderqueue);

#endif /* CE_RENDERQUEUE_HPP */
