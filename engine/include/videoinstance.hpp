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

#ifndef CE_VIDEOINSTANCE_HPP
#define CE_VIDEOINSTANCE_HPP

#include "commonheaders.hpp"
#include "semaphore.hpp"
#include "mmpfile.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "soundobject.hpp"
#include "videoresource.hpp"

namespace cursedearth
{
    class video_instance_t final: boost::noncopyable
    {
        enum class state_t {
            stopped,
            stopping,
            paused,
            playing
        };

        static const size_t s_cache_size = 8;

    public:
        video_instance_t(sound_object_t, ce_video_resource*);
        ~video_instance_t();

        void advance(float elapsed);
        void progress(int percents);
        void render();

        bool is_stopped();
        void play();
        void pause();
        void stop();

    private:
        void do_advance();
        void execute();

    private:
        const sound_object_t m_object;
        ce_video_resource* m_resource;
        state_t m_state = state_t::stopped;
        int m_frame = -1;
        float m_play_time = 0.0f, m_sync_time = 0.0f; // playing/synchronization time in seconds
        ce_texture* m_texture;
        ce_material* m_material;
        ce_mmpfile* m_rgba_frame;
        ce_mmpfile* m_ycbcr_frames[s_cache_size];
        semaphore_ptr_t m_prepared_frames;
        semaphore_ptr_t m_unprepared_frames;
        std::atomic<bool> m_done;
        std::thread m_thread;
    };

    typedef std::shared_ptr<video_instance_t> video_instance_ptr_t;
}

#endif
