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

import cetools
import cemadplay
import cempg123
import cempg321

def tune_madplay(env):
	env.Replace(
		MADPLAYFLAGS="--verbose --display-time=remaining",
		MADPLAYTYPE="wave",
		MADPLAYTARGET="$MP32WAVTARGET",

		MP32WAVCOM="$MADPLAYCOM",
		MP32WAVCOMSTR="$MADPLAYCOMSTR",
	)

	env["BUILDERS"]["Mp32Wav"] = env["BUILDERS"]["MadPlay"]

def tune_mpg12321(env):
	env.Replace(
		MPG123FLAGS="--verbose -w $MP32WAVTARGET",

		MP32WAVCOM="$MPG123COM",
		MP32WAVCOMSTR="$MPG123COMSTR",
	)

	env["BUILDERS"]["Mp32Wav"] = env["BUILDERS"]["Mpg123"]

codecs = {
	cemadplay.UTILITY: (cemadplay, tune_madplay),
	cempg123.UTILITY: (cempg123, tune_mpg12321),
	cempg321.UTILITY: (cempg321, tune_mpg12321),
}

def generate(env):
	env.SetDefault(
		MP32WAVCODEC="auto",
		MP32WAVTARGET="$TARGET",
	)

	cetools.generate_codec("mp32wav", codecs, "$MP32WAVCODEC", env)

def exists(env):
	return cetools.codec_exists(codecs, env)
