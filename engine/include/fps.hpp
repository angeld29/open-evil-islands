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

#ifndef CE_FPS_HPP
#define CE_FPS_HPP

#include "commonheaders.hpp"

namespace cursedearth
{
    class fps_t final: untransferable_t
    {
    public:
        fps_t();

        void advance(float elapsed);

        const std::string& text() { return m_text; }

    private:
        int m_frame_count = 0;
        float m_time = 0.0f;
        std::string m_text;
    };

    typedef std::shared_ptr<fps_t> fps_ptr_t;

    fps_ptr_t make_fps();
}

#endif
