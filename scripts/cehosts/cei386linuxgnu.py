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

def configure_release_mode(env):
	env.AppendUnique(CXXFLAGS=["-O2", "-w"])

def configure_debug_mode(env):
	env.AppendUnique(CXXFLAGS=["-g", "-Wall", "-Wextra"])

configure_build_mode = {
	"release": configure_release_mode,
	"debug": configure_debug_mode,
}

def configure(env):
	env.AppendUnique(CXXFLAGS=["-pipe"])
	configure_build_mode[env["BUILD_MODE"]](env)

def get_tools():
	return ["gnulink", "gcc", "g++", "gas", "ar"]

"""
env.AppendUnique(
	CFLAGS=["-std=c99"],
	CPPFLAGS=["-pipe", "-pedantic-errors"],
)
if env["PLATFORM"] == "posix":
	env.AppendUnique(
		CPPDEFINES=["_GNU_SOURCE"],
	)
"""
