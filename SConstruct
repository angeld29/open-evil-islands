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

EnsurePythonVersion(2, 5)
EnsureSConsVersion(1, 2)

import site

site.addsitedir("scripts")

import ceenv

env = ceenv.create_environment()

Export("env")

engine = Alias("engine", SConscript(dirs="engine"))
spikes = Alias("spikes", SConscript(dirs="spikes"))

Depends(spikes, engine)
Default(spikes)

targets = [engine, spikes]

if "mp32oga" in COMMAND_LINE_TARGETS:
    targets.append(Alias("mp32oga", SConscript("Mp32Oga.SConscript")))

if "bik2ogv" in COMMAND_LINE_TARGETS:
    targets.append(Alias("bik2ogv", SConscript("Bik2Ogv.SConscript")))

Alias("all", targets)
