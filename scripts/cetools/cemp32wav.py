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

import SCons

import ceerrors
import cemadplay

def generate(env):
	env.SetDefault(
		MP32WAVTARGET="$TARGET",
	)

	if not cemadplay.exists(env):
		ceerrors.interrupt("madplay not found")

	cemadplay.generate(env)

	env.Replace(
		MADPLAYFLAGS="--verbose --display-time=remaining",
		MADPLAYTYPE="wave",
		MADPLAYTARGET="$MP32WAVTARGET",
	)

	env.Append(
		MP32WAVCOM="$MADPLAYCOM",
		MP32WAVCOMSTR="$MADPLAYCOMSTR",
	)

def exists(env):
	return ceffmpeg.exists(env)
