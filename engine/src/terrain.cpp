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

#include <cstdio>
#include <cmath>
#include <vector>

#include "alloc.hpp"
#include "logging.hpp"
#include "event.hpp"
#include "optionmanager.hpp"
#include "rendersystem.hpp"
#include "texturemanager.hpp"
#include "mprhelpers.hpp"
#include "mprrenderitem.hpp"
#include "terrain.hpp"

namespace cursedearth
{
    void ce_terrain_load_tile_mmpfiles(ce_terrain* terrain)
    {
        std::vector<char> name(terrain->mprfile->name->length + 3 + 1);
        for (int i = 0; i < terrain->mprfile->texture_count; ++i) {
            snprintf(name.data(), name.size(), "%s%03d", terrain->mprfile->name->str, i);
            ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile_from_resources(name.data());
            ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
            ce_vector_push_back(terrain->tile_mmpfiles, mmpfile);
        }
    }

    void ce_terrain_load_tile_textures(ce_terrain* terrain)
    {
        std::vector<char> name(terrain->mprfile->name->length + 3 + 1);
        for (int i = 0; i < terrain->mprfile->texture_count; ++i) {
            snprintf(name.data(), name.size(), "%s%03d", terrain->mprfile->name->str, i);
            ce_texture* texture = ce_texture_add_ref(ce_texture_manager_get(name.data()));
            ce_texture_wrap(texture, CE_TEXTURE_WRAP_CLAMP_TO_EDGE);
            ce_vector_push_back(terrain->tile_textures, texture);
        }
    }

    void ce_terrain_sector_react(ce_event* event)
    {
        ce_terrain_sector* sector = (ce_terrain_sector*)((ce_event_ptr*)event->impl)->ptr;

        if (ce_option_manager->terrain_tiling) {
            sector->texture = ce_texture_add_ref(ce_texture_manager_get("default0"));

            // tile textures are necessary for geometry creation
            ce_once_exec(sector->terrain->tile_once, (void(*)())ce_terrain_load_tile_textures, sector->terrain);
        } else {
            sector->texture = ce_texture_new(sector->name->str, sector->mmpfile);

            // we do not more need in mmp file (we already have a texture)
            ce_mmpfile_del(sector->mmpfile);
            sector->mmpfile = NULL;

            // TODO: ???
            //ce_texture_manager_put(ce_texture_add_ref(sector->texture));
        }

        ce_texture_wrap(sector->texture, CE_TEXTURE_WRAP_CLAMP_TO_EDGE);

        sector->renderlayer = ce_rendergroup_get(sector-> terrain->rendergroups[sector->water], sector->texture);
        sector->renderitem = ce_mprrenderitem_new(sector->terrain->mprfile, sector->x, sector->z, sector->water, sector->terrain->tile_textures);

        ce_mpr_get_aabb(&sector->renderitem->aabb, sector->terrain->mprfile, sector->x, sector->z, sector->water);

        sector->renderitem->position = CE_VEC3_ZERO;
        sector->renderitem->orientation = CE_QUAT_IDENTITY;
        sector->renderitem->bbox.aabb = sector->renderitem->aabb;
        sector->renderitem->bbox.axis = CE_QUAT_IDENTITY;

        ce_scenenode_add_renderitem(sector->scenenode, sector->renderitem);

        if (++sector->terrain->completed_job_count == sector->terrain->queued_job_count) {
            // free tile mmp files to avoid extra memory usage
            ce_vector_for_each(sector->terrain->tile_mmpfiles, (void(*)(void*))ce_mmpfile_del);
            ce_vector_clear(sector->terrain->tile_mmpfiles);

            ce_logging_info("terrain: done loading `%s'", sector->terrain->mprfile->name->str);

            // tile textures are necessary for geometry rendering if tiling do not touch it
        }
    }

    void ce_terrain_sector_exec(ce_terrain_sector* sector)
    {
        if (!ce_option_manager->terrain_tiling) {
            sector->mmpfile = ce_texture_manager_open_mmpfile_from_cache(sector->name->str);

            if (NULL == sector->mmpfile || sector->mmpfile->version < CE_MMPFILE_VERSION || sector->mmpfile->user_info < CE_MPR_TEXTURE_VERSION) {
                ce_mmpfile_del(sector->mmpfile);

                // lazy loading tile mmp files
                ce_once_exec(sector->terrain->tile_once, (void(*)())ce_terrain_load_tile_mmpfiles, sector->terrain);

                sector->mmpfile = ce_mpr_generate_texture(sector->terrain->mprfile, sector->terrain->tile_mmpfiles, sector->x, sector->z, sector->water);

                // force to DXT1?
                ce_mmpfile_convert(sector->mmpfile, CE_MMPFILE_FORMAT_DXT1);

                if (ce_option_manager->texture_caching) {
                    ce_texture_manager_save_mmpfile(sector->name->str, sector->mmpfile);
                }
            }
        }

        ce_event_manager_post_ptr(ce_render_system_thread_id(), ce_terrain_sector_react, sector);
    }

    void ce_scenenode_updated(void* listener)
    {
        ce_terrain_sector* sector = (ce_terrain_sector*)listener;
        ce_renderlayer_add(sector->renderlayer, sector->renderitem);
    }

    ce_terrain_sector* ce_terrain_sector_new(ce_terrain* terrain, const char* name, int x, int z, bool water)
    {
        ce_terrain_sector* sector = (ce_terrain_sector*)ce_alloc_zero(sizeof(ce_terrain_sector));
        sector->name = ce_string_new_str(name);
        sector->x = x;
        sector->z = z;
        sector->water = water;
        sector->scenenode = ce_scenenode_new(terrain->scenenode);
        sector->scenenode->listener = { NULL, NULL, NULL, ce_scenenode_updated, NULL, sector };
        sector->terrain = terrain;

        ce_thread_pool_enqueue((void(*)())ce_terrain_sector_exec, sector);

        return sector;
    }

    void ce_terrain_sector_del(ce_terrain_sector* sector)
    {
        if (NULL != sector) {
            ce_texture_del(sector->texture);
            ce_mmpfile_del(sector->mmpfile);
            ce_string_del(sector->name);
            ce_free(sector, sizeof(ce_terrain_sector));
        }
    }

    ce_terrain* ce_terrain_new(ce_mprfile* mprfile, ce_renderqueue* renderqueue, const vector3_t* position, const quaternion_t* orientation, ce_scenenode* scenenode)
    {
        ce_logging_info("terrain: loading `%s'...", mprfile->name->str);

        ce_terrain* terrain = (ce_terrain*)ce_alloc_zero(sizeof(ce_terrain));
        terrain->mprfile = mprfile;
        terrain->materials[CE_MPRFILE_MATERIAL_LAND] = ce_mpr_create_material(mprfile, false);
        terrain->materials[CE_MPRFILE_MATERIAL_WATER] = ce_mpr_create_material(mprfile, true);
        terrain->rendergroups[CE_MPRFILE_MATERIAL_LAND] = ce_renderqueue_get(renderqueue, 0, terrain->materials[CE_MPRFILE_MATERIAL_LAND]);
        terrain->rendergroups[CE_MPRFILE_MATERIAL_WATER] = ce_renderqueue_get(renderqueue, 100, terrain->materials[CE_MPRFILE_MATERIAL_WATER]);
        terrain->tile_mmpfiles = ce_vector_new_reserved(mprfile->texture_count);
        terrain->tile_textures = ce_vector_new_reserved(mprfile->texture_count);
        terrain->tile_once = ce_once_new();
        terrain->sectors = ce_vector_new_reserved(2 * mprfile->sector_x_count * mprfile->sector_z_count);
        terrain->scenenode = ce_scenenode_new(scenenode);
        terrain->scenenode->position = *position;
        terrain->scenenode->orientation = *orientation;

        std::vector<char> name(terrain->mprfile->name->length + 3 + 3 + 1 + 1);

        for (int i = 0; i < CE_MPRFILE_MATERIAL_COUNT; ++i) {
            bool water = CE_MPRFILE_MATERIAL_WATER == i;
            for (int z = 0; z < terrain->mprfile->sector_z_count; ++z) {
                for (int x = 0; x < terrain->mprfile->sector_x_count; ++x) {
                    ce_mprsector* mpr_sector = terrain->mprfile->sectors + z * terrain->mprfile->sector_x_count + x;
                    if (water && NULL == mpr_sector->water_allow) {
                        // do not add empty geometry
                        continue;
                    }

                    snprintf(name.data(), name.size(), water ? "%s%03d%03dw" : "%s%03d%03d", terrain->mprfile->name->str, x, z);
                    ce_vector_push_back(terrain->sectors, ce_terrain_sector_new(terrain, name.data(), x, z, water));

                    ++terrain->queued_job_count;
                }
            }
        }

        ce_logging_info("terrain: %zu jobs queued", terrain->queued_job_count);
        return terrain;
    }

    void ce_terrain_del(ce_terrain* terrain)
    {
        if (NULL != terrain) {
            ce_scenenode_del(terrain->scenenode);
            ce_vector_for_each(terrain->sectors, (void(*)(void*))ce_terrain_sector_del);
            ce_vector_del(terrain->sectors);
            ce_once_del(terrain->tile_once);
            ce_vector_for_each(terrain->tile_textures, (void(*)(void*))ce_texture_del);
            ce_vector_del(terrain->tile_textures);
            ce_vector_for_each(terrain->tile_mmpfiles, (void(*)(void*))ce_mmpfile_del);
            ce_vector_del(terrain->tile_mmpfiles);
            ce_material_del(terrain->materials[CE_MPRFILE_MATERIAL_WATER]);
            ce_material_del(terrain->materials[CE_MPRFILE_MATERIAL_LAND]);
            ce_mprfile_close(terrain->mprfile);
            ce_free(terrain, sizeof(ce_terrain));
        }
    }

    ce_scenenode* ce_terrain_find_scenenode(ce_terrain* terrain, float x, float z)
    {
        // FIXME: opengl hard-code
        z = fabsf(z);

        size_t sector_x = static_cast<int>(x) / (CE_MPRFILE_VERTEX_SIDE - 1);
        size_t sector_z = static_cast<int>(z) / (CE_MPRFILE_VERTEX_SIDE - 1);

        return (ce_scenenode*)terrain->scenenode->childs->items[sector_z * terrain->mprfile->sector_x_count + sector_x];
    }
}
