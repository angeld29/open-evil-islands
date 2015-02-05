#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  This file is part of Cursed Earth.
#
#  Cursed Earth is an open source, cross-platform port of Evil Islands.
#  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see <http://www.gnu.org/licenses/>.

def configure_release_mode(env):
    env.AppendUnique(
        CCFLAGS=["-O2", "-w"],
        LINKFLAGS=["-s"],
    )

def configure_debug_mode(env):
    env.AppendUnique(
        CCFLAGS=["-ggdb", "-Wall", "-Wextra"],
    )

configure_build_mode = {
    "release": configure_release_mode,
    "debug": configure_debug_mode,
}

def configure(env):
    env["CE_GCC_BIT"] = True

    env.AppendUnique(
        CFLAGS=["-std=c11"],     # C11   ISO/IEC 9899:2011
        CXXFLAGS=["-std=c++11"], # C++11 ISO/IEC 14882:2011
        CCFLAGS=["-pipe", "-pedantic-errors"],
    )

    configure_build_mode[env["BUILD_MODE"]](env)
