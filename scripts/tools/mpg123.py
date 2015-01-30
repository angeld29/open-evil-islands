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

import SCons

UTILITY = "mpg123"

def detect(env):
    return env.WhereIs(UTILITY)

def generate(env):
    env.SetDefault(
        MPG123=detect(env),

        MPG123FLAGS="",
        MPG123SOURCE="$SOURCE",

        MPG123COM="$MPG123 $MPG123FLAGS $MPG123SOURCE",
        MPG123COMSTR="",

        MPG123PREFIX="",
        MPG123SUFFIX="",
        MPG123SRCSUFFIX="",
    )

    env.Append(
        BUILDERS={
            "Mpg123": SCons.Builder.Builder(
                action=SCons.Action.Action("$MPG123COM", "$MPG123COMSTR"),
                prefix="$MPG123PREFIX",
                suffix="$MPG123SUFFIX",
                src_suffix="$MPG123SRCSUFFIX",
                single_source=True,
            ),
        },
    )

def exists(env):
    return detect(env)
