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
import cemadplay
import cempg123
import cempg321

def tune_madplay(env):
	env.Replace(
		MADPLAYFLAGS="--verbose --display-time=remaining",
		MADPLAYTYPE="wave",
		MADPLAYTARGET="$MP32WAVTARGET",
	)

	env.Append(
		MP32WAVCOM="$MADPLAYCOM",
		MP32WAVCOMSTR="$MADPLAYCOMSTR",
	)

	env["BUILDERS"]["Mp32Wav"] = env["BUILDERS"]["MadPlay"]

def tune_mpg123(env):
	env.Replace(
		MPG123FLAGS="--verbose -w $MP32WAVTARGET",
	)

	env.Append(
		MP32WAVCOM="$MPG123COM",
		MP32WAVCOMSTR="$MPG123COMSTR",
	)

	env["BUILDERS"]["Mp32Wav"] = env["BUILDERS"]["Mpg123"]

MODULE, TUNE = range(2)

codecs = {
	cemadplay.UTILITY: (cemadplay, tune_madplay),
	cempg123.UTILITY: (cempg123, tune_mpg123),
	cempg321.UTILITY: (cempg321, tune_mpg123),
}

def generate(env):
	env.SetDefault(
		MP32WAVCODEC="",
		MP32WAVTARGET="$TARGET",
	)

	codec = env.subst("$MP32WAVCODEC")

	if "auto" != codec:
		if not codecs[codec][MODULE].exists(env):
			ceerrors.interrupt("mp32wav: codec '%s' not found", codec)
	else:
		for key, value in codecs.iteritems():
			codec = key
			if value[MODULE].exists(env):
				break
			logging.warning("mp32wav: codec '%s' not found" % codec)

	if not codecs[codec][MODULE].exists(env):
		ceerrors.interrupt("mp32wav: no appropriate codec found")

	logging.info("mp32wav: using '%s' codec" % codec)

	codecs[codec][MODULE].generate(env)
	codecs[codec][TUNE](env)

def exists(env):
	for value in codecs.itervalues():
		if value[MODULE].exists(env):
			return True
	return False
