# -*- coding: utf-8 -*-

# This file is part of Open Evil Islands.
#
# Open Evil Islands is an open source, cross-platform port of the original Evil Islands from Nival.
# Copyright (C) 2009-2017 Yanis Kurganov <yanis.kurganov@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

engine = Alias("engine", SConscript(dirs="engine"))
spikes = Alias("spikes", SConscript(dirs="spikes"))
tools = Alias("tools", SConscript(dirs="tools"))

Depends(spikes, engine)

targets = [engine, spikes, tools]

Return("targets")
