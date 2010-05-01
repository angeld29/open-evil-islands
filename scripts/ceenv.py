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
import logging
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

logging_levels = {
	"debug": logging.DEBUG,
	"info": logging.INFO,
	"warning": logging.WARNING,
	"error": logging.ERROR,
	"critical": logging.CRITICAL,
}

def create_environment():
	defenv = SCons.Defaults.DefaultEnvironment()
	topdir = defenv.Dir('#').get_abspath()

	config = ConfigParser.SafeConfigParser()
	config.read(os.path.join(topdir, cfg)
				for cfg in ("cursedearth.cfg", "cursedearth_local.cfg"))

	config_get = lambda sec, opt, default=None: config.get(sec, opt) \
							if config.has_option(sec, opt) else default
	path_list_builder = lambda sec, opt, default="": [os.path.normpath(path)
		for path in config_get(sec, opt, default).split(';') if len(path) > 0]

	variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)

	variables.Add(SCons.Variables.EnumVariable("HOST",
		"Build for HOST",
		config_get("CE", "HOST", cehosts.defaults[defenv["PLATFORM"]]),
		cehosts.hosts.keys()))

	variables.Add(SCons.Variables.EnumVariable("GRAPHICS_LIBRARY",
		"Select graphics library",
		config_get("CE", "GRAPHICS_LIBRARY", cegraphlibs.defaults[defenv["PLATFORM"]]),
		cegraphlibs.graphlibs.keys()))

	variables.Add(SCons.Variables.BoolVariable("RELEASE",
		"Build the project in release mode", config_get("CE", "RELEASE", "yes")))

	variables.Add(SCons.Variables.EnumVariable("LOGGING_LEVEL",
		"Select logging level",
		config_get("CE", "LOGGING_LEVEL", "info"),
		logging_levels.keys()))

	env = SCons.Environment.Environment(
		variables=variables,
		tools=[],
		toolpath=[os.path.join("#scripts", "cetools")]
	)

	logging.basicConfig(
		level=logging_levels[env["LOGGING_LEVEL"]],
		format="%(levelname)s: %(message)s",
		datefmt="%a, %d %b %Y %H:%M:%S",
	)

	env["BUILD_MODE"] = "release" if env["RELEASE"] else "debug"
	env["GEN_PATH"] = os.path.join("$HOST", "$BUILD_MODE")

	logging.info("The build was started for:")
	logging.info("\thost: '%s' (%s)", env["HOST"],
		cehosts.hosts[env["HOST"]].get_description())
	logging.info("\tgraphics library: '%s' (%s)", env["GRAPHICS_LIBRARY"],
		cegraphlibs.graphlibs[env["GRAPHICS_LIBRARY"]].get_description())

	cehosts.hosts[env["HOST"]].configure(env)
	cegraphlibs.graphlibs[env["GRAPHICS_LIBRARY"]].configure(env)

	if env["RELEASE"]:
		env.AppendUnique(CPPDEFINES=["NDEBUG"])

	env.AppendUnique(
		CPPPATH=path_list_builder(env["HOST"], "ADDITIONAL_INCLUDE_PATHS"),
		LIBPATH=path_list_builder(env["HOST"], "ADDITIONAL_LIBRARY_PATHS"),
		CPPDEFINES=path_list_builder(env["HOST"], "ADDITIONAL_DEFINES"),
		LIBS=path_list_builder(env["HOST"], "ADDITIONAL_LIBS"),
	)

	env.Help(variables.GenerateHelpText(env))

	return env
