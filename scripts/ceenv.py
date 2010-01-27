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

import ceposix
import cewin32
import cewin32cross
import cedarwin

targets = {
	"posix": ceposix,
	"win32": cewin32,
	"win32cross": cewin32cross,
	"darwin": cedarwin,
}

def create_environment():
	defenv = SCons.Defaults.DefaultEnvironment()
	topdir = defenv.Dir('#').get_abspath()

	config = ConfigParser.SafeConfigParser()
	config.read(os.path.join(topdir, cfg)
				for cfg in ("cursedearth.cfg", "cursedearth_local.cfg"))

	split_by_sep = lambda name: name.split(';')
	path_builder = lambda sec, opt: os.path.normpath(config.get(sec, opt))
	path_list_builder = lambda sec, opt: [os.path.normpath(path) for path
		in split_by_sep(config.get(sec, opt))]

	variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)

	target = config.get("CE", "CETARGET") \
			if config.has_option("CE", "CETARGET") else defenv["PLATFORM"]

	variables.Add(EnumVariable("CETARGET",
		"Build target", target, targets.keys()))

	variables.Add(SCons.Variables.BoolVariable("CERELEASE",
		"Build the project in release mode",
		config.get("CE", "CERELEASE")))

	variables.Add("CEINCPATHS",
		"Additional include directories (semicolon-separated list of names)",
		raw_path_list_builder("CE", "CEINCPATHS")
		if config.has_option("CE", "CEINCPATHS") else [])

	variables.Add("CELIBPATHS",
		"Additional library directories (semicolon-separated list of names)",
		raw_path_list_builder("CE", "CELIBPATHS")
		if config.has_option("CE", "CELIBPATHS") else [])

	variables.Add("CELIBS",
		"Additional libraries (semicolon-separated list of names)",
		split_by_sep(config.get("CE", "CELIBS"))
		if config.has_option("CE", "CELIBS") else [])

	env = SCons.Environment.Environment(variables=variables,
		tools=targets[].get_tools(),
		toolpath=[os.path.join("#scripts", "tools")])

	env["BUILD_MODE"] = "release" if env["RELEASE"] else "debug"

	configurations[env["PLATFORM"]].configure(env)

	env.AppendUnique(
		CPPPATH=["$QFLAC_INCPATHS"],
		LIBPATH=["$QFLAC_LIBPATHS"],
		LIBS=["$QFLAC_LIBS"],
		CXXFLAGS=["$QFLAC_CXXFLAGS"],
	)

	env["CEHELP"] = variables.GenerateHelpText(env)

	return env
