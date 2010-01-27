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

import site

site.addsitedir("scripts")

import ceenv

env = ceenv.create_environment()

Export("env")

variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)
variables.Add(SCons.Variables.BoolVariable("RELEASE",
	"Build the project in release mode", "no"))

env["COMPILER"] = "gcc"
env["GRAPHLIB"] = "opengl"

env.AppendUnique(
	CFLAGS=["-std=c99"],
	CPPFLAGS=["-pipe", "-pedantic-errors"],
)

env.AppendUnique(
	CPPFLAGS=["-O2", "-w"] if env["RELEASE"] else ["-g", "-Wall", "-Wextra"],
)

if env["RELEASE"]:
	env.AppendUnique(
		CPPDEFINES=["NDEBUG"],
	)

if env["PLATFORM"] == "posix":
	env.AppendUnique(
		CPPDEFINES=["_GNU_SOURCE"],
	)

if env["PLATFORM"] == "win32":
	env.AppendUnique(
		CPPDEFINES=["FREEGLUT_STATIC"],
	)

if env["GRAPHLIB"] == "opengl":
	env.AppendUnique(
		CPPDEFINES=["GL_GLEXT_PROTOTYPES"],
	)

game = env.Alias("game", env.SConscript(dirs="game"))
spikes = env.Alias("spikes", env.SConscript(dirs="spikes"))

env.Depends(spikes, game)

env.Default(env.Alias("all", [game, spikes]))

Help(env["CEHELP"])
