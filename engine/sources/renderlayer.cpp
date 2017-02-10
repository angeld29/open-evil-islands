/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "alloc.hpp"
#include "rendersystem.hpp"
#include "renderlayer.hpp"

namespace cursedearth
{
    ce_renderlayer* ce_renderlayer_new(ce_texture* texture)
    {
        ce_renderlayer* renderlayer = (ce_renderlayer*)ce_alloc(sizeof(ce_renderlayer));
        renderlayer->texture = texture;
        renderlayer->renderitems = ce_vector_new();
        return renderlayer;
    }

    void ce_renderlayer_del(ce_renderlayer* renderlayer)
    {
        if (NULL != renderlayer) {
            ce_vector_del(renderlayer->renderitems);
            ce_free(renderlayer, sizeof(ce_renderlayer));
        }
    }

    void ce_renderlayer_clear(ce_renderlayer* renderlayer)
    {
        ce_vector_clear(renderlayer->renderitems);
    }

    void ce_renderlayer_add(ce_renderlayer* renderlayer, ce_renderitem* renderitem)
    {
        ce_vector_push_back(renderlayer->renderitems, renderitem);
    }

    void ce_renderlayer_render(ce_renderlayer* renderlayer)
    {
        if (!ce_vector_empty(renderlayer->renderitems)) {
            ce_texture_bind(renderlayer->texture);
            for (size_t i = 0; i < renderlayer->renderitems->count; ++i) {
                ce_renderitem* renderitem = (ce_renderitem*)renderlayer->renderitems->items[i];
                if (renderitem->visible) {
                    ce_render_system_apply_transform(&renderitem->world_position, &renderitem->world_orientation, &CE_VEC3_UNIT_SCALE);
                    ce_renderitem_render(renderitem);
                    ce_render_system_discard_transform();
                }
            }
            ce_texture_unbind(renderlayer->texture);
        }
    }
}
