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

#include "alloc.hpp"
#include "utility.hpp"
#include "logging.hpp"
#include "root.hpp"

namespace cursedearth
{
    class map_viewer_t final: public scene_manager_t
    {
    public:
        map_viewer_t(const input_context_const_ptr_t& input_context, const ce_optparse_ptr_t& option_parser):
            scene_manager_t(input_context, nullptr),
            m_input_supply(std::make_shared<input_supply_t>(input_context)),
            m_anmfps_inc_event(m_input_supply->repeat(m_input_supply->push(input_button_t::kb_add))),
            m_anmfps_dec_event(m_input_supply->repeat(m_input_supply->push(input_button_t::kb_subtract)))
        {
            const char* zone;
            bool only_mpr;

            ce_optparse_get(option_parser, "zone", &zone);
            ce_optparse_get(option_parser, "only_mpr", &only_mpr);

            load_mpr(zone);
            if (!only_mpr) {
                load_mob(zone);
            }

            // play random animations
            /*srand(time(NULL));
            for (size_t i = 0; i < ce_figure_manager->entities->count; ++i) {
                ce_figentity* figentity = static_cast<ce_figentity*>(ce_figure_manager->entities->items[i]);
                int anm_count = ce_figentity_get_animation_count(figentity);
                if (anm_count > 0) {
                    const char* name = ce_figentity_get_animation_name(figentity, rand() % anm_count);
                    ce_figentity_play_animation(figentity, name);
                }
            }*/

            /*if (NULL != get_terrain()) {
                vector3_t position;
                ce_camera_set_position(ce_root::instance()->scenemng->camera, ce_vec3_init(&position, 0.0f, ce_root::instance()->scenemng->terrain->mprfile->max_y, 0.0f));
                ce_camera_yaw_pitch(ce_root::instance()->scenemng->camera, deg2rad(45.0f), deg2rad(30.0f));
            }*/
        }

    private:
        virtual void do_advance(float elapsed) final
        {
            m_input_supply->advance(elapsed);

            float animation_fps = root_t::instance()->animation_fps;

            if (m_anmfps_inc_event->triggered()) animation_fps += 1.0f;
            if (m_anmfps_dec_event->triggered()) animation_fps -= 1.0f;

            if (m_text_timeout > 0.0f) {
                m_text_timeout -= elapsed;
            }

            if (animation_fps != root_t::instance()->animation_fps) {
                m_text_timeout = 3.0f;
            }

            m_text_color.a = clamp(m_text_timeout, 0.0f, 1.0f);
            root_t::instance()->animation_fps = clamp(animation_fps, 1.0f, 50.0f);
        }

        virtual void do_render() final
        {
            if (m_text_timeout > 0.0f) {
                std::string text = str(boost::format("Animation FPS: %1%") % root_t::instance()->animation_fps);
                viewport_t viewport = get_viewport();
                ce_font* font = get_font();

                ce_font_render(font, (viewport.width - ce_font_get_width(font, text)) / 2,
                    1 * (viewport.height - ce_font_get_height(font)) / 5, m_text_color, text);
            }
        }

    private:
        float m_text_timeout = 0.0f;
        color_t m_text_color = CE_COLOR_CORNFLOWER;
        input_supply_ptr_t m_input_supply;
        input_event_const_ptr_t m_anmfps_inc_event;
        input_event_const_ptr_t m_anmfps_dec_event;
    };

    scene_manager_ptr_t make_scene_manager(const input_context_const_ptr_t& input_context,
                                           const AIDirectorPointerType&,
                                           const ce_optparse_ptr_t& option_parser)
    {
        return make_unique<map_viewer_t>(input_context, option_parser);
    }
}

int main(int argc, char* argv[])
{
    using namespace cursedearth;
    ce_alloc_init();
    try {
        ce_optparse_ptr_t option_parser = option_manager_t::make_parser();

        ce_optparse_add(option_parser, "help", CE_TYPE_BOOL, NULL, false, "h", "help", "display this help and exit");
        ce_optparse_add(option_parser, "version", CE_TYPE_BOOL, NULL, false, "v", "version", "display version information and exit");

        ce_optparse_set_standard_properties(option_parser, CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
            "Cursed Earth: Map Viewer", "This program is part of Cursed Earth spikes.\nMap Viewer - explore Evil Islands zones.");

        ce_optparse_add(option_parser, "only_mpr", CE_TYPE_BOOL, NULL, false, NULL, "only-mpr", "without objects (do not load mob)");
        ce_optparse_add(option_parser, "zone", CE_TYPE_STRING, NULL, true, NULL, NULL, "any ZONE.mpr file in `EI/Maps'");
        ce_optparse_add_control(option_parser, "+/-", "change animation FPS");

        root_t root(option_parser, argc, argv);
        return root.exec();
    } catch (const std::exception& error) {
        ce_logging_fatal("map viewer: %s", error.what());
    } catch (...) {
        ce_logging_fatal("map viewer: unknown error");
    }
    return EXIT_FAILURE;
}

#if 0
    const char* ei_path2;
    ce_optparse_get(option_parser, "ei_path2", &ei_path2);

    const char* zone;
    ce_optparse_get(option_parser, "zone", &zone);

    char path[512];
    snprintf(path, sizeof(path), "%s/Camera/%s.cam",
        ei_path2, zone /*"mainmenu"*/ /*"camera01"*/);

    ce_mem_file* mem_file = ce_mem_file_new_path(path);
    if (NULL != mem_file) {
        ce_cam_file* cam_file = ce_cam_file_new(mem_file);

        for (size_t i = 0; i < cam_file->record_count; ++i) {
            printf("%u %u\n", cam_file->records[i].time, cam_file->records[i].unknown);
            printf("%f %f %f\n", cam_file->records[i].position[0], cam_file->records[i].position[1], cam_file->records[i].position[2]);
            printf("%f %f %f %f\n", cam_file->records[i].rotation[0], cam_file->records[i].rotation[1], cam_file->records[i].rotation[2], cam_file->records[i].rotation[3]);
            printf("-----\n");
        }

        ce_vec3 position;
        ce_vec3_init_array(&position, cam_file->records[0].position);
        ce_swap_temp(float, &position.z, &position.y);
        position.z = -position.z;

        ce_camera_set_position(ce_root::instance()->scenemng->camera, &position);

        ce_quat orientation, temp, temp2, temp3;
        ce_quat_init_array(&temp, cam_file->records[0].rotation);

        ce_quat_init_polar(&temp2, ce_deg2rad(90), &CE_VEC3_UNIT_X);
        ce_quat_mul(&temp3, &temp2, &temp);

        ce_quat_init_polar(&temp2, ce_deg2rad(180), &CE_VEC3_UNIT_Y);
        ce_quat_mul(&orientation, &temp2, &temp3);

        ce_quat_conj(&orientation, &orientation);

        ce_camera_set_orientation(ce_root::instance()->scenemng->camera, &orientation);
        ce_camera_set_orientation(ce_root::instance()->scenemng->camera, &temp);

        ce_cam_file_del(cam_file);
        ce_mem_file_del(mem_file);
    } else {
        ce_logging_error("map viewer: could not open file `%s'", path);
    }
#endif
