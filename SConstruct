#!/usr/bin/env python
# -*- coding: utf-8 -*-

import SCons.Script

variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)
variables.Add(SCons.Variables.BoolVariable("RELEASE",
	"Build the project in release mode", "no"))

env = Environment(variables=variables)

env.AppendUnique(
	CPPDEFINES=["_GNU_SOURCE"],
	CFLAGS=["-std=c99"],
	CPPFLAGS=["-pipe", "-pedantic-errors"],
)

env.AppendUnique(
	CPPFLAGS=["-O2", "-w"] if env["RELEASE"] else ["-g", "-Wall", "-Wextra"],
)

env.AppendUnique(
	CPPDEFINES=["GL_GLEXT_PROTOTYPES", "CE_NEED_STRRPBRK", "CE_NEED_STRLCPY"],
)

Export("env")

game = env.Alias("game", env.SConscript(dirs="game"))
spikes = env.Alias("spikes", env.SConscript(dirs="spikes"))

env.Depends(spikes, game)

env.Default(env.Alias("all", [game, spikes]))

Help(variables.GenerateHelpText(env))
