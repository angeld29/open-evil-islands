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

import os.path
import ConfigParser

import SCons.Environment
import SCons.Script
import SCons.Defaults
import SCons.Variables
import SCons.Variables.BoolVariable
import SCons.Variables.PathVariable
import SCons.Variables.ListVariable
import SCons.Variables.EnumVariable

import cehosts
import cegraphlibs

def create_environment():
	defenv = SCons.Defaults.DefaultEnvironment()
	topdir = defenv.Dir('#').get_abspath()

	config = ConfigParser.SafeConfigParser()
	config.read(os.path.join(topdir, cfg)
				for cfg in ("cursedearth.cfg", "cursedearth_local.cfg"))

	args_get = lambda opt, default=None: args.get(opt, default)
	config_get = lambda opt, default=None: config.get("CE", opt) \
								if config.has_option("CE", opt) else default

	variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)

	variables.Add(SCons.Variables.EnumVariable("HOST",
		"Build for HOST",
		config_get("HOST", cehosts.defaults[defenv["PLATFORM"]]),
		cehosts.hosts.keys()))

	variables.Add(SCons.Variables.EnumVariable("GRAPHICS_LIBRARY",
		"Select graphics library",
		config_get("GRAPHICS_LIBRARY", cegraphlibs.defaults[defenv["PLATFORM"]]),
		cegraphlibs.graphlibs.keys()))

	variables.Add(SCons.Variables.BoolVariable("RELEASE",
		"Build the project in release mode", config_get("RELEASE", "yes")))

	variables.Add("ADDITIONAL_INCLUDE_PATHS",
		"Additional include directories (semicolon-separated list of names)",
		config_get("ADDITIONAL_INCLUDE_PATHS", []))

	variables.Add("ADDITIONAL_LIBRARY_PATHS",
		"Additional library directories (semicolon-separated list of names)",
		config_get("ADDITIONAL_LIBRARY_PATHS", []))

	variables.Add("ADDITIONAL_LIBS",
		"Additional libraries (semicolon-separated list of names)",
		config_get("ADDITIONAL_LIBS", []))

	env = SCons.Environment.Environment(variables=variables,
		toolpath=os.path.join("#scripts", "cetools")) # TODO: tools=[] ?

	env["BUILD_MODE"] = "release" if env["RELEASE"] else "debug"
	env["GEN_PATH"] = os.path.join("$HOST", "$BUILD_MODE")

	cehosts.hosts[env["HOST"]].configure(env)
	cegraphlibs.graphlibs[env["GRAPHICS_LIBRARY"]].configure(env)

	if env["RELEASE"]:
		env.AppendUnique(CPPDEFINES=["NDEBUG"])

	"""
	split_by_sep = lambda name: name.split(';')
	path_builder = lambda sec, opt: os.path.normpath(config.get(sec, opt))
	path_list_builder = lambda sec, opt: [os.path.normpath(path) for path
		in split_by_sep(config.get(sec, opt))]
	env.AppendUnique(
		CPPPATH=[],
		LIBPATH=[],
		LIBS=[],
	)
	"""

	env.Help(variables.GenerateHelpText(env))

	return env
