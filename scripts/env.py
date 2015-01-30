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
import logging
import ConfigParser

import SCons

import hosts
import tools.mp32wav
import tools.wav2oga
import tools.bik2ogv

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
    config.read(os.path.join(topdir, cfg) for cfg in ("cursedearth.cfg", "cursedearth_local.cfg"))

    config_get = lambda sec, opt, default=None: config.get(sec, opt) if config.has_option(sec, opt) else default
    path_list_builder = lambda sec, opt, default="": [os.path.normpath(path) for path in config_get(sec, opt, default).split(';') if len(path) > 0]

    variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)

    variables.Add(SCons.Variables.EnumVariable("HOST",
        "Build for HOST",
        config_get("CE", "HOST", hosts.defaults[defenv["PLATFORM"]]),
        hosts.hosts.keys()))

    variables.Add(SCons.Variables.BoolVariable("RELEASE",
        "Build the project in release mode",
        config_get("CE", "RELEASE", "yes")))

    variables.Add(SCons.Variables.EnumVariable("LOGGING_LEVEL",
        "Select logging level",
        config_get("CE", "LOGGING_LEVEL", "info"),
        logging_levels.keys()))

    variables.Add(SCons.Variables.PathVariable("MP3_PATH",
        "Set the input path for MP3 files (for mp32oga target)",
        config_get("CE", "MP3_PATH", "."),
        SCons.Variables.PathVariable.PathIsDir))

    variables.Add(SCons.Variables.PathVariable("OGA_PATH",
        "Set the output path for OGA files (for mp32oga target)",
        config_get("CE", "OGA_PATH", "."),
        SCons.Variables.PathVariable.PathIsDirCreate))

    variables.Add(SCons.Variables.EnumVariable("MP3_CODEC",
        "Select MP3 codec to use (for mp32oga target)",
        config_get("CE", "MP3_CODEC", "auto"),
        ["auto"] + tools.mp32wav.codecs.keys()))

    variables.Add(SCons.Variables.EnumVariable("OGA_CODEC",
        "Select OGA codec to use (for mp32oga target)",
        config_get("CE", "OGA_CODEC", "auto"),
        ["auto"] + tools.wav2oga.codecs.keys()))

    variables.Add(SCons.Variables.EnumVariable("OGA_QUALITY",
        "Set the quality for ogg vorbis encoder in kbit/s (for mp32oga target)",
        config_get("CE", "OGA_QUALITY", "4"), [str(n) for n in xrange(11)]))

    variables.Add(SCons.Variables.PathVariable("BIK_PATH",
        "Set the input path for BIK files (for bik2ogv target)",
        config_get("CE", "BIK_PATH", "."),
        SCons.Variables.PathVariable.PathIsDir))

    variables.Add(SCons.Variables.PathVariable("OGV_PATH",
        "Set the output path for OGV files (for bik2ogv target)",
        config_get("CE", "OGV_PATH", "."),
        SCons.Variables.PathVariable.PathIsDirCreate))

    variables.Add(SCons.Variables.EnumVariable("OGV_CODEC",
        "Select OGV codec to use (for bik2ogv target)",
        config_get("CE", "OGV_CODEC", "auto"),
        ["auto"] + tools.bik2ogv.codecs.keys()))

    variables.Add("OGV_VIDEO_BITRATE", "Set the video bitrate for "
        "ogg theora encoder in kbit/s (for bik2ogv target)",
        config_get("CE", "OGV_VIDEO_BITRATE", "5000"))

    variables.Add("OGV_AUDIO_BITRATE", "Set the audio bitrate for "
        "ogg theora encoder in kbit/s (for bik2ogv target)",
        config_get("CE", "OGV_AUDIO_BITRATE", "128"))

    env = SCons.Environment.Environment(
        variables=variables,
        tools=[],
        toolpath=[os.path.join("#scripts", "tools")]
    )

    logging.basicConfig(
        level=logging_levels[env["LOGGING_LEVEL"]],
        format="%(levelname)s: %(message)s",
        datefmt="%a, %d %b %Y %H:%M:%S",
    )

    env["CE_GENERIC_BIT"] = True

    env["BUILD_MODE"] = "release" if env["RELEASE"] else "debug"
    env["GEN_PATH"] = os.path.join("$HOST", "$BUILD_MODE")

    hosts.hosts[env["HOST"]].configure(env)

    if env["RELEASE"]:
        env.AppendUnique(CPPDEFINES=["NDEBUG"])

    env.AppendUnique(
        CPPPATH=path_list_builder("CE", "ADDITIONAL_INCLUDE_PATHS"),
        LIBPATH=path_list_builder("CE", "ADDITIONAL_LIBRARY_PATHS"),
        CPPDEFINES=path_list_builder("CE", "ADDITIONAL_DEFINES"),
        LIBS=path_list_builder("CE", "ADDITIONAL_LIBS"),
    )

    env.AppendUnique(
        CPPPATH=path_list_builder(env["HOST"], "ADDITIONAL_INCLUDE_PATHS"),
        LIBPATH=path_list_builder(env["HOST"], "ADDITIONAL_LIBRARY_PATHS"),
        CPPDEFINES=path_list_builder(env["HOST"], "ADDITIONAL_DEFINES"),
        LIBS=path_list_builder(env["HOST"], "ADDITIONAL_LIBS"),
    )

    env.Help(variables.GenerateHelpText(env))

    return env
