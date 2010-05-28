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

import cetools
import ceffmpeg
import cemencoder

def tune_ffmpeg(env):
	env.Replace(
		FFMPEGSUFFIX=".ogv",
		FFMPEGSRCSUFFIX=".bik",
		FFMPEGFLAGS="-vcodec libtheora -acodec libvorbis -f ogg "
						"-b ${BIK2OGVVIDEOBPS}K -ab ${BIK2OGVAUDIOBPS}K",
	)

	env["BUILDERS"]["Bik2Ogv"] = env["BUILDERS"]["FFmpeg"]

def tune_mencoder(env):
	env.Replace(
		MENCODERSUFFIX=".ogv",
		MENCODERSRCSUFFIX=".bik",
		MENCODERFLAGS="-ovc lavc -oac lavc -lavcopts vcodec=libtheora:acodec=libvorbis:"
						"vbitrate=${BIK2OGVVIDEOBPS}K:abitrate=${BIK2OGVAUDIOBPS}K",
	)

	env["BUILDERS"]["Bik2Ogv"] = env["BUILDERS"]["MEncoder"]

codecs = {
	ceffmpeg.UTILITY: (ceffmpeg, tune_ffmpeg),
	#cemencoder.UTILITY: (cemencoder, tune_mencoder), # not tested
}

def generate(env):
	env.SetDefault(
		BIK2OGVCODEC="auto",
		BIK2OGVVIDEOBPS="200",
		BIK2OGVAUDIOBPS="64",
	)

	cetools.generate_codec("bik2ogv", codecs, "$BIK2OGVCODEC", env)

def exists(env):
	return cetools.codec_exists(codecs, env)
