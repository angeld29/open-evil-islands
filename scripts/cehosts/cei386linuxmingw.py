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

import logging

import ceerrors
import cetools.cemingwcross as mingw
import cecompilers.cegcc as gcc

def get_description():
	return "Minimalist GNU win32 (cross) compiler"

def configure(env):
	if env["PLATFORM"] != "posix":
		ceerrors.interrupt("This host is available only on Linux.")

	# prefer MinGW on Linux for Windows
	if not mingw.exists(env):
		ceerrors.interrupt("Could not locate the mingw32 cross compiler. "
			"Please, install 'mingw' package. On ubuntu, for example, install "
			"'mingw32', 'mingw32-runtime' and 'mingw32-binutils' packages.")

	logging.info("mingw: using '%s' the mingw32 cross compiler", mingw.detect(env))
	mingw.generate(env)

	env["CPU_TYPE"] = "i386"
	env["TARGET_PLATFORM"] = "win32"

	gcc.configure(env)

	env.AppendUnique(
		CPPDEFINES=[
			"WINVER=0x0501",       # Windows XP required
			"WIN32_LEAN_AND_MEAN", # excludes some stuff like Cryptography,
								   # DDE, RPC, Shell, and Windows Sockets
			"NOCOMM",              # excludes the serial communication API
		],
		CPPFLAGS=["-mthreads"],  # specifies that MinGW-specific
		LINKFLAGS=["-mthreads"], # thread support is to be used
	)
