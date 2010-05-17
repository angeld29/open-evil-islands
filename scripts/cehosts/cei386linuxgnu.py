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

import SCons.Tool

import ceerrors
import cecompilers.cegcc as gcc

def get_description():
	return "The GNU C/C++ compiler for Linux"

def configure(env):
	if env["PLATFORM"] != "posix":
		ceerrors.interrupt("This host is available only on Linux.")

	# prefer GNU tools on Linux
	for tool in ("gnulink", "gcc", "g++", "gas", "ar"):
		SCons.Tool.Tool(tool)(env)

	env["CPU_TYPE"] = "i386"
	env["TARGET_PLATFORM"] = "posix"

	gcc.configure(env)

	env.AppendUnique(
		CPPDEFINES=[
			# IEEE Std 1003.1-2004, Open Group Single UNIX Specification, version 3
			# includes POSIX.1-2001 and XPG6 things
			"_XOPEN_SOURCE=600",
			"_GNU_SOURCE", # TODO: do not use not portable code
		],
		CPPFLAGS=["-pthread"],  # add support for multithreading
		LINKFLAGS=["-pthread"], # using the POSIX threads library
	)
