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

import SCons.Tool
import SCons.Util

def find(env):
    key_name = "powerpc-linux-gnu"
    key_program = key_name + "-gcc"
    key_program = env.WhereIs(key_program) or SCons.Util.WhereIs(key_program)
    return key_name if key_program is not None else None

def generate(env):
    for tool in ("gnulink", "gcc", "g++", "gas", "ar"):
        SCons.Tool.Tool(tool)(env)

    base_name = find(env) or ""

    env["CC"] = base_name + "-gcc"
    env["CXX"] = base_name + "-g++"
    env["AS"] = base_name + "-as"
    env["RC"] = base_name + "-windres"
    env["AR"] = base_name + "-ar"
    env["RANLIB"] = base_name + "-ranlib"

def exists(env):
    return find(env)
