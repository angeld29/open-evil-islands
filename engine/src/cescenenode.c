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

#include <stdio.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cerendersystem.h"
#include "ceroot.h"
#include "cescenenode.h"

ce_scenenode* ce_scenenode_new(ce_scenenode* parent)
{
    ce_scenenode* scenenode = ce_alloc_zero(sizeof(ce_scenenode));
    scenenode->culled = true;
    scenenode->position = CE_VEC3_ZERO;
    scenenode->orientation = CE_QUAT_IDENTITY;
    scenenode->renderitems = ce_vector_new();
    scenenode->parent = parent;
    scenenode->childs = ce_vector_new();
    if (NULL != parent) {
        ce_scenenode_attach_child(parent, scenenode);
    }
    return scenenode;
}

void ce_scenenode_del(ce_scenenode* scenenode)
{
    if (NULL != scenenode) {
        ce_scenenode_detach_from_parent(scenenode);
        for (size_t i = 0; i < scenenode->childs->count; ++i) {
            ce_scenenode* child = scenenode->childs->items[i];
            child->parent = NULL;
            ce_scenenode_del(child);
        }
        ce_vector_del(scenenode->childs);
        ce_occlusion_del(scenenode->occlusion);
        ce_vector_for_each(scenenode->renderitems, ce_renderitem_del);
        ce_vector_del(scenenode->renderitems);
        if (NULL != scenenode->listener.destroyed) {
            (*scenenode->listener.destroyed)(scenenode->listener.listener);
        }
        ce_free(scenenode, sizeof(ce_scenenode));
    }
}

void ce_scenenode_detach_from_parent(ce_scenenode* scenenode)
{
    if (NULL != scenenode->parent) {
        ce_scenenode_detach_child(scenenode->parent, scenenode);
        if (NULL != scenenode->listener.detached) {
            (*scenenode->listener.detached)(scenenode->listener.listener);
        }
    }
}

void ce_scenenode_attach_child(ce_scenenode* scenenode, ce_scenenode* child)
{
    child->parent = scenenode;
    ce_vector_push_back(scenenode->childs, child);
}

void ce_scenenode_detach_child(ce_scenenode* scenenode, ce_scenenode* child)
{
    child->parent = NULL;
    ce_vector_remove_all(scenenode->childs, child);
}

void ce_scenenode_add_renderitem(ce_scenenode* scenenode,
                                ce_renderitem* renderitem)
{
    ce_vector_push_back(scenenode->renderitems, renderitem);
}

int ce_scenenode_count_visible_cascade(ce_scenenode* scenenode)
{
    if (scenenode->culled) {
        return 0;
    }

    int count = 1;
    for (size_t i = 0; i < scenenode->childs->count; ++i) {
        count += ce_scenenode_count_visible_cascade(scenenode->childs->items[i]);
    }

    return count;
}

static void ce_scenenode_update_transform(ce_scenenode* scenenode)
{
    if (NULL == scenenode->parent) {
        // absolute state == relative state
        scenenode->world_position = scenenode->position;
        scenenode->world_orientation = scenenode->orientation;
    } else {
        ce_vec3_rot(&scenenode->world_position,
                    &scenenode->position,
                    &scenenode->parent->world_orientation);
        ce_vec3_add(&scenenode->world_position,
                    &scenenode->world_position,
                    &scenenode->parent->world_position);
        ce_quat_mul(&scenenode->world_orientation,
                    &scenenode->parent->world_orientation,
                    &scenenode->orientation);
    }

    for (size_t i = 0; i < scenenode->renderitems->count; ++i) {
        ce_renderitem* renderitem = scenenode->renderitems->items[i];
        ce_vec3_rot(&renderitem->world_position,
                    &renderitem->position,
                    &scenenode->world_orientation);
        ce_vec3_add(&renderitem->world_position,
                    &renderitem->world_position,
                    &scenenode->world_position);
        ce_quat_mul(&renderitem->world_orientation,
                    &scenenode->world_orientation,
                    &renderitem->orientation);
    }
}

static void ce_scenenode_update_bounds(ce_scenenode* scenenode)
{
    ce_bbox_clear(&scenenode->world_bbox);

    for (size_t i = 0; i < scenenode->renderitems->count; ++i) {
        ce_renderitem* renderitem = scenenode->renderitems->items[i];
        if (renderitem->visible) {
            renderitem->world_bbox.aabb = renderitem->bbox.aabb;

            ce_vec3_rot(&renderitem->world_bbox.aabb.origin,
                        &renderitem->world_bbox.aabb.origin,
                        &scenenode->world_orientation);
            ce_vec3_add(&renderitem->world_bbox.aabb.origin,
                        &renderitem->world_bbox.aabb.origin,
                        &scenenode->world_position);
            ce_quat_mul(&renderitem->world_bbox.axis,
                        &scenenode->world_orientation,
                        &renderitem->bbox.axis);

            ce_bbox_merge(&scenenode->world_bbox, &renderitem->world_bbox);
        }
    }

    for (size_t i = 0; i < scenenode->childs->count; ++i) {
        ce_scenenode* child = scenenode->childs->items[i];
        ce_bbox_merge(&scenenode->world_bbox, &child->world_bbox);
    }
}

void ce_scenenode_update_force_cascade(ce_scenenode* scenenode)
{
    scenenode->culled = false;

    if (NULL != scenenode->listener.about_to_update) {
        (*scenenode->listener.about_to_update)(scenenode->listener.listener);
    }

    ce_scenenode_update_transform(scenenode);
    for (size_t i = 0; i < scenenode->childs->count; ++i) {
        ce_scenenode_update_force_cascade(scenenode->childs->items[i]);
    }
    ce_scenenode_update_bounds(scenenode);

    if (NULL != scenenode->listener.updated) {
        (*scenenode->listener.updated)(scenenode->listener.listener);
    }
}

void ce_scenenode_update_cascade(ce_scenenode* scenenode, const ce_frustum* frustum)
{
    // try to cull scene node BEFORE update for performance reasons
    // rendering defects are possible, such as culling partially visible objects

    // step 1 - frustum culling
    scenenode->culled = !ce_frustum_test_bbox(frustum, &scenenode->world_bbox);

    // step 2 - HW occlusion test
    if (!scenenode->culled && NULL != scenenode->occlusion) {
        scenenode->culled = !ce_occlusion_query(scenenode->occlusion,
                                                &scenenode->world_bbox);
    }

    if (!scenenode->culled) {
        if (NULL != scenenode->listener.about_to_update) {
            (*scenenode->listener.about_to_update)(scenenode->listener.listener);
        }

        ce_scenenode_update_transform(scenenode);
        for (size_t i = 0; i < scenenode->childs->count; ++i) {
            ce_scenenode_update_cascade(scenenode->childs->items[i], frustum);
        }
        ce_scenenode_update_bounds(scenenode);

        if (NULL != scenenode->listener.updated) {
            (*scenenode->listener.updated)(scenenode->listener.listener);
        }
    }
}

static void ce_scenenode_draw_bbox(const ce_bbox* bbox)
{
    ce_render_system_apply_transform(&bbox->aabb.origin,
                                    &bbox->axis, &bbox->aabb.extents);
    ce_render_system_draw_wire_cube();
    ce_render_system_discard_transform();
}

void ce_scenenode_draw_bboxes_cascade(ce_scenenode* scenenode)
{
    if (!scenenode->culled) {
        ce_scenenode_draw_bbox(&scenenode->world_bbox);

        if (!ce_root.comprehensive_bbox_only) {
            for (size_t i = 0; i < scenenode->renderitems->count; ++i) {
                ce_renderitem* renderitem = scenenode->renderitems->items[i];
                if (renderitem->visible) {
                    ce_scenenode_draw_bbox(&renderitem->world_bbox);
                }
            }
        }

        for (size_t i = 0; i < scenenode->childs->count; ++i) {
            ce_scenenode_draw_bboxes_cascade(scenenode->childs->items[i]);
        }
    }
}
