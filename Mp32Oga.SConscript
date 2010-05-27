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

Import("env")

env = env.Clone(
	tools=["cemp32wav", "cewav2oga"],
	MP32WAVTARGET="${TARGET.base}.wav",
	WAV2OGASOURCE="$MP32WAVTARGET",
	WAV2OGAQUALITY="$OGA_QUALITY",
)

targets = [
	env.Command(
		os.path.join("$OGA_PATH", os.path.splitext(node.name)[0] + ".oga"),
		node,
		[
			Action("$MP32WAVCOM", "$MP32WAVCOMSTR"),
			Action("$WAV2OGACOM", "$WAV2OGACOMSTR"),
			Delete("$MP32WAVTARGET"),
		]
	) for node in env.Glob(os.path.join("$MP3_PATH", "*.mp3"))
]

Return("targets")
