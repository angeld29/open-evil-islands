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
#include "logging.hpp"
#include "soundobject.hpp"
#include "videoobject.hpp"
#include "root.hpp"

namespace cursedearth
{
    class media_player_t final: public scene_manager_t
    {
    public:
        media_player_t(const input_context_const_ptr_t& input_context, const ce_optparse_ptr_t& option_parser):
            scene_manager_t(input_context),
            m_input_supply(std::make_shared<input_supply_t>(input_context)),
            m_pause_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_space)))
        {
            const char* track;
            ce_optparse_get(option_parser, "track", &track);
            ce_optparse_get(option_parser, "only_sound", &m_only_sound);

            m_track.assign(track);

            if (m_only_sound) {
                play_sound_object(m_sound_object = make_sound_object(m_track));
            } else {
                play_video_object(m_video_object = make_video_object(m_track));
            }

            if (0 == m_sound_object && 0 == m_video_object) {
                ce_logging_error("media player: could not play track `%s'", m_track.c_str());
            }
        }

    private:
        virtual void do_advance(float elapsed) final
        {
            m_input_supply->advance(elapsed);
            sound_object_advance(m_sound_object, elapsed);
            video_object_advance(m_video_object, elapsed);

            if (m_pause_event->triggered()) {
                m_paused = !m_paused;
                if (m_paused) {
                    pause_sound_object(m_sound_object);
                    pause_video_object(m_video_object);
                } else {
                    play_sound_object(m_sound_object);
                    play_video_object(m_video_object);
                }
            }

            if (m_only_sound) {
                if (sound_object_is_valid(m_sound_object) && !sound_object_is_stopped(m_sound_object)) {
                    m_text_color.a += elapsed * m_alpha_sign;
                    m_alpha_sign = m_text_color.a < 0.25f ? 1.0f : (m_text_color.a > 1.0f ? -1.0f : m_alpha_sign);
                } else {
                    m_text_color.a = 1.0f;
                }
            }
        }

        virtual void do_render() final
        {
            if (m_only_sound) {
                std::string text;

                if (sound_object_is_valid(m_sound_object)) {
                    if (sound_object_is_stopped(m_sound_object)) {
                        text = str(boost::format("Track `%1%' stopped") % m_track);
                    } else {
                        text = str(boost::format("Playing track `%1%'") % m_track);
                    }
                } else {
                    text = str(boost::format("Unable to play track `%1%'") % m_track);
                }

                viewport_t viewport = get_viewport();
                ce_font* font = get_font();
                ce_font_render(font, (viewport.width - ce_font_get_width(font, text)) / 2,
                    1 * (viewport.height - ce_font_get_height(font)) / 5, m_text_color, text);
            }

            video_object_render(m_video_object);
        }

    private:
        bool m_only_sound = false;
        bool m_paused = false;
        float m_alpha_sign = -1.0f;
        std::string m_track;
        color_t m_text_color = CE_COLOR_CORNFLOWER;
        input_supply_ptr_t m_input_supply;
        input_event_const_ptr_t m_pause_event;
        sound_object_t m_sound_object;
        video_object_t m_video_object;
    };

    scene_manager_ptr_t make_scene_manager(const input_context_const_ptr_t& input_context, const ce_optparse_ptr_t& option_parser)
    {
        return make_unique<media_player_t>(input_context, option_parser);
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
            "Cursed Earth: Media Player", "This program is part of Cursed Earth spikes.\nMedia Player - play Evil Islands sounds and videos.");

        ce_optparse_add(option_parser, "only_sound", CE_TYPE_BOOL, NULL, false, NULL, "only-sound", "do not try to play video");
        ce_optparse_add(option_parser, "track", CE_TYPE_STRING, NULL, true, NULL, NULL, "TRACK.* file in `EI/Stream' or TRACK.* file in `EI/Movies' or internal resource");

        root_t root(option_parser, argc, argv);
        return root.exec();
    } catch (const std::exception& error) {
        ce_logging_fatal("media player: %s", error.what());
    }
    return EXIT_FAILURE;
}
