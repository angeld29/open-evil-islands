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

import SCons.Errors

import cetools.cemingwcross as mingw

def get_description():
	return "Minimalist GNU win32 (cross) compiler. " \
			"A Linux hosted, win32 target, cross compiler for C/C++."

def configure_release_mode(env):
	env.AppendUnique(
		CCFLAGS=["-O2", "-w"],
	)

def configure_debug_mode(env):
	env.AppendUnique(
		CCFLAGS=["-g", "-Wall", "-Wextra"],
	)

configure_build_mode = {
	"release": configure_release_mode,
	"debug": configure_debug_mode,
}

def configure(env):
	if env["PLATFORM"] != "posix":
		raise SCons.Errors.StopError("This host is available only on Linux.")

	print "Checking for mingw32 cross compiler...",
	if mingw.exists(env):
		print mingw.find(env) + "."
	else:
		print "not found."
		raise SCons.Errors.StopError("Could not locate mingw32 cross compiler.")

	mingw.generate(env)

	env["OS"] = "win32"
	env["COMPILER"] = "gcc"

	env.AppendUnique(
		CFLAGS=["-std=c99"],
		CCFLAGS=["-pedantic-errors"],
	)

	configure_build_mode[env["BUILD_MODE"]](env)
