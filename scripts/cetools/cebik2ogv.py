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
import ceffmpeg

def generate(env):
	if not ceffmpeg.exists(env):
		ceerrors.interrupt("ffmpeg not found")

	ceffmpeg.generate(env)

	env.SetDefault(
		OGVVIDEOBPS="200",
		OGVAUDIOBPS="64",
	)

	env.Replace(
		FFMPEGFLAGS="-vcodec libtheora -acodec libvorbis -f ogg "
						"-b ${OGVVIDEOBPS}K -ab ${OGVAUDIOBPS}K",

		FFMPEGSUFFIX=".ogv",
		FFMPEGSRCSUFFIX=".bik",
	)

def exists(env):
	return ceffmpeg.exists(env)
