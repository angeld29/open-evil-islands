#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  This file is part of Cursed Earth.
#
#  Cursed Earth is an open source, cross-platform port of Evil Islands.
#  Copyright (C) 2009-2010 Yanis Kurganov.
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

import os

import SCons.Util

def detect(env):
    for variants in ((cpu, kernel) for cpu in xrange(3, 7) for kernel in ("", "msvc")):
        key_name = "i%d86-mingw32%s" % variants
        key_program = key_name + "-gcc"
        key_program = env.WhereIs(key_program) or SCons.Util.WhereIs(key_program)
        if key_program is not None:
            return key_name
    return None

def generate(env):
    base_name = detect(env) or "mingw32"

    env["CC"] = base_name + "-gcc"
    env["CXX"] = base_name + "-g++"
    env["AS"] = base_name + "-as"
    env["RC"] = base_name + "-windres"
    env["AR"] = base_name + "-ar"
    env["RANLIB"] = base_name + "-ranlib"

    env["SHLIBSUFFIX"] = ".dll"
    env["PROGSUFFIX"] = ".exe"

    env.AppendUnique(
        CPPPATH=[os.path.join(os.sep, "usr", "local", base_name, "include")],
        LIBPATH=[os.path.join(os.sep, "usr", "local", base_name, "lib")],
    )

def exists(env):
    return detect(env)
