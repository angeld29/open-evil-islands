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
#include "figuremanager.hpp"
#include "root.hpp"

namespace cursedearth
{
    class figure_viewer_t final: public scene_manager_t
    {
    public:
        figure_viewer_t(const input_context_const_ptr_t& input_context, const ce_optparse_ptr_t& option_parser):
            scene_manager_t(input_context, nullptr),
            m_input_supply(std::make_shared<input_supply_t>(input_context)),
            m_strength_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_1))),
            m_dexterity_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_2))),
            m_height_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_3))),
            m_change_animation_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_a))),
            m_animation_fps_plus_event(m_input_supply->repeat(m_input_supply->push(input_button_t::kb_add))),
            m_animation_fps_minus_event(m_input_supply->repeat(m_input_supply->push(input_button_t::kb_subtract)))
        {
            const char *primary_texture, *secondary_texture, *animation_name, *figure_name;
            ce_optparse_get(option_parser, "primary_texture", &primary_texture);
            ce_optparse_get(option_parser, "secondary_texture", &secondary_texture);
            ce_optparse_get(option_parser, "animation_name", &animation_name);
            ce_optparse_get(option_parser, "figure", &figure_name);

            if (NULL != primary_texture) {
                m_primary_texture.assign(primary_texture);
            }
            if (NULL != secondary_texture) {
                m_secondary_texture.assign(secondary_texture);
            }
            if (NULL != animation_name) {
                m_animation_name.assign(animation_name);
            }
            if (NULL != figure_name) {
                m_figure_name.assign(figure_name);
            }

            if (update_figure()) {
                if (!m_animation_name.empty()) {
                    if (ce_figentity_play_animation(m_figure, animation_name)) {
                        int count = ce_figentity_get_animation_count(m_figure);
                        for (m_animation_index = 0; m_animation_index < count; ++m_animation_index) {
                            if (m_animation_name == ce_figentity_get_animation_name(m_figure, m_animation_index)) {
                                break;
                            }
                        }
                    } else {
                        ce_logging_warning("figure viewer: could not play animation `%s'", animation_name);
                    }
                }
            }

            //vector3_t position;
            //ce_camera_set_position(ce_root::instance()->scenemng->camera, ce_vec3_init(&position, 0.0f, 2.0f, -4.0f));

            //ce_camera_set_near(ce_root::instance()->scenemng->camera, 0.1f);
            //ce_camera_yaw_pitch(ce_root::instance()->scenemng->camera, deg2rad(180.0f), deg2rad(30.0f));

            //ce_root::instance()->scenemng->camera_move_sensitivity = 2.5f;
            //ce_root::instance()->scenemng->camera_zoom_sensitivity = 0.5f;
        }

    private:
        void set_text(const boost::format& format)
        {
            m_text = str(format);
            m_text_timeout = 3.0f;
            m_text_color.a = 1.0f;
        }

        bool update_figure()
        {
            ce_figure_manager_remove_entity(m_figure);

            vector3_t position = CE_VEC3_ZERO;

            quaternion_t orientation, q1, q2;
            ce_quat_init_polar(&q1, deg2rad(180.0f), &CE_VEC3_UNIT_Z);
            ce_quat_init_polar(&q2, deg2rad(270.0f), &CE_VEC3_UNIT_X);
            ce_quat_mul(&orientation, &q2, &q1);

            const char *parts[1] = { NULL }, *textures[3] = { m_primary_texture.c_str(), m_secondary_texture.c_str(), NULL };

            m_figure = ce_figure_manager_create_entity(m_figure_name.c_str(), &m_complection, &position, &orientation, parts, textures);
            if (NULL == m_figure) {
                return false;
            }

            add_node(m_figure->scenenode);

            if (-1 != m_animation_index) {
                ce_figentity_play_animation(m_figure, ce_figentity_get_animation_name(m_figure, m_animation_index));
            }

            return true;
        }

        virtual void do_advance(float elapsed) final
        {
            m_input_supply->advance(elapsed);

            if (m_text_timeout > 0.0f) {
                m_text_timeout -= elapsed;
                m_text_color.a = clamp(m_text_timeout, 0.0f, 1.0f);
            }

            float animation_fps = root_t::instance()->animation_fps;

            if (m_animation_fps_plus_event->triggered()) animation_fps += 1.0f;
            if (m_animation_fps_minus_event->triggered()) animation_fps -= 1.0f;

            if (animation_fps != root_t::instance()->animation_fps) {
                root_t::instance()->animation_fps = clamp(animation_fps, 1.0f, 50.0f);
                set_text(boost::format("Animation FPS: %1%") % root_t::instance()->animation_fps);
            }

            bool need_update_figure = false;

            if (m_strength_event->triggered()) {
                if ((m_complection.strength += 0.1f) >= 1.1f) {
                    m_complection.strength = 0.0f;
                }
                need_update_figure = true;
                set_text(boost::format("Strength: %.2f") % m_complection.strength);
            }

            if (m_dexterity_event->triggered()) {
                if ((m_complection.dexterity += 0.1f) >= 1.1f) {
                    m_complection.dexterity = 0.0f;
                }
                need_update_figure = true;
                set_text(boost::format("Dexterity: %.2f") % m_complection.dexterity);
            }

            if (m_height_event->triggered()) {
                if ((m_complection.height += 0.1f) >= 1.1f) {
                    m_complection.height = 0.0f;
                }
                need_update_figure = true;
                set_text(boost::format("Height: %.2f") % m_complection.height);
            }

            if (need_update_figure) {
                update_figure();
            }

            if (m_change_animation_event->triggered()) {
                ce_figentity_stop_animation(m_figure);
                if (++m_animation_index == ce_figentity_get_animation_count(m_figure)) {
                    m_animation_index = -1;
                }
                if (-1 != m_animation_index) {
                    const char* name = ce_figentity_get_animation_name(m_figure, m_animation_index);
                    ce_figentity_play_animation(m_figure, name);
                    set_text(boost::format("Animation name: `%1%'") % name);
                } else {
                    set_text(boost::format("No animation"));
                }
            }
        }

        virtual void do_render() final
        {
            if (m_text_timeout > 0.0f) {
                viewport_t viewport = get_viewport();
                ce_font* font = get_font();
                ce_font_render(font, (viewport.width - ce_font_get_width(font, m_text)) / 2,
                    1 * (viewport.height - ce_font_get_height(font)) / 5, m_text_color, m_text);
            }
        }

    private:
        int m_animation_index = -1;
        complection_t m_complection = { 1.0f, 1.0f, 1.0f };
        color_t m_text_color = CE_COLOR_CORNFLOWER;
        float m_text_timeout;
        std::string m_text;
        std::string m_primary_texture;
        std::string m_secondary_texture;
        std::string m_animation_name;
        std::string m_figure_name;
        ce_figentity* m_figure = NULL;
        input_supply_ptr_t m_input_supply;
        input_event_const_ptr_t m_strength_event;
        input_event_const_ptr_t m_dexterity_event;
        input_event_const_ptr_t m_height_event;
        input_event_const_ptr_t m_change_animation_event;
        input_event_const_ptr_t m_animation_fps_plus_event;
        input_event_const_ptr_t m_animation_fps_minus_event;
    };

    scene_manager_ptr_t make_scene_manager(const input_context_const_ptr_t& input_context,
                                           const AIDirectorPointerType&,
                                           const ce_optparse_ptr_t& option_parser)
    {
        return make_unique<figure_viewer_t>(input_context, option_parser);
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
            "Cursed Earth: Figure Viewer", "This program is part of Cursed Earth spikes.\nFigure Viewer - explore Evil Islands figures.");

        ce_optparse_add(option_parser, "primary_texture", CE_TYPE_STRING, "default0", false, NULL, "primary-texture", "primary texture");
        ce_optparse_add(option_parser, "secondary_texture", CE_TYPE_STRING, "default0", false, NULL, "secondary-texture", "secondary texture");
        ce_optparse_add(option_parser, "animation_name", CE_TYPE_STRING, NULL, false, NULL, "animation-name", "play animation with specified name");
        ce_optparse_add(option_parser, "figure", CE_TYPE_STRING, NULL, true, NULL, NULL, "internal figure name");

        ce_optparse_add_control(option_parser, "+/-", "change animation FPS");
        ce_optparse_add_control(option_parser, "a", "play next animation");
        ce_optparse_add_control(option_parser, "1", "change strength");
        ce_optparse_add_control(option_parser, "2", "change dexterity");
        ce_optparse_add_control(option_parser, "3", "change height");

        root_t root(option_parser, argc, argv);
        return root.exec();
    } catch (const std::exception& error) {
        ce_logging_fatal("figure viewer: %s", error.what());
    } catch (...) {
        ce_logging_fatal("figure viewer: unknown error");
    }
    return EXIT_FAILURE;
}
