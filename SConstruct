#!/usr/bin/env python
# -*- coding: utf-8 -*-

import SCons.Script

variables = SCons.Variables.Variables(args=SCons.Script.ARGUMENTS)
variables.Add(SCons.Variables.BoolVariable("RELEASE",
	"Build the project in release mode", "no"))

env = Environment(variables=variables)

env.AppendUnique(
	CPPFLAGS=["-pipe"] + (["-O2", "-w"] if env["RELEASE"]
						else ["-g", "-Wall", "-Wextra"]),
)

env.AppendUnique(
	CPPDEFINES=["CE_NEED_STRRPBRK", "CE_NEED_STRLCPY"],
)

Export("env")

core = env.Alias("core", env.SConscript(dirs="core"))
test = env.Alias("test", env.SConscript(dirs="test"))

env.Depends(test, core)

env.Default(env.Alias("all", [core, test]))

Help(variables.GenerateHelpText(env))
