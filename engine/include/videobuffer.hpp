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

#ifndef CE_VIDEOBUFFER_HPP
#define CE_VIDEOBUFFER_HPP

#include "ringbuffer.hpp"
#include "videooptions.hpp"
#include "mmpfile.hpp"

namespace cursedearth
{
    class video_buffer_t final: untransferable_t
    {
    public:
        video_buffer_t():
            m_buffer(video_options_t::frame_count)
        {
        }

        size_t read_available() const { return m_buffer.read_available(); }

        void push(const mmpfile_ptr_t& frame) { m_buffer.push(frame); }

        bool try_pop(mmpfile_ptr_t& frame)
        {
            return m_buffer.pop(frame, false);
        }

        mmpfile_ptr_t acquire_from_cache(size_t width, size_t height)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::ignore = lock;

            if (m_frames.empty()) {
                return mmpfile_ptr_t(ce_mmpfile_new(width, height, 1, CE_MMPFILE_FORMAT_YCBCR, 0), mmpfile_dtor_t());
            }

            mmpfile_ptr_t frame = m_frames.back();
            m_frames.pop_back();

            return std::move(frame);
        }

        void release_to_cache(const mmpfile_ptr_t& frame)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::ignore = lock;
            m_frames.push_back(frame);
        }

    private:
        std::mutex m_mutex;
        std::vector<mmpfile_ptr_t> m_frames;
        ring_buffer_t<mmpfile_ptr_t> m_buffer;
    };

    typedef std::shared_ptr<video_buffer_t> video_buffer_ptr_t;

    inline video_buffer_ptr_t make_video_buffer()
    {
        return std::make_shared<video_buffer_t>();
    }
}

#endif
