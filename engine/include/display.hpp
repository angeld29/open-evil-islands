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

#ifndef CE_DISPLAY_HPP
#define CE_DISPLAY_HPP

#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>

namespace cursedearth
{
    enum display_rotation_t
    {
        DISPLAY_ROTATION_NONE = 0,
        DISPLAY_ROTATION_0    = 1,
        DISPLAY_ROTATION_90   = 2,
        DISPLAY_ROTATION_180  = 4,
        DISPLAY_ROTATION_270  = 8
    };

    enum display_reflection_t
    {
        DISPLAY_REFLECTION_NONE = 0,
        DISPLAY_REFLECTION_X    = 1,
        DISPLAY_REFLECTION_Y    = 2
    };

    display_rotation_t display_rotation_from_degrees(int value);
    display_reflection_t display_reflection_from_bool(bool x, bool y);

    struct display_mode_t
    {
        int width, height;
        int bpp, rate;
    };

    typedef std::shared_ptr<class display_manager_t> display_manager_ptr_t;
    typedef std::shared_ptr<const class display_manager_t> display_manager_const_ptr_t;

    class display_manager_t: boost::noncopyable
    {
    public:
        display_manager_t();
        virtual ~display_manager_t() = default;

        void enter(size_t index, display_rotation_t, display_reflection_t);
        void exit();

        void dump_supported_modes_to_stdout();
        void dump_supported_rotations_to_stdout();
        void dump_supported_reflections_to_stdout();

    private:
        virtual void do_enter(int width, int height, int bpp, int rate, display_rotation_t, display_reflection_t) = 0;
        virtual void do_exit() = 0;

    private:
        display_rotation_t m_supported_rotation;
        display_reflection_t m_supported_reflection;
        std::vector<display_mode_t> m_supported_modes;
    };
}

#endif /* CE_DISPLAY_HPP */
