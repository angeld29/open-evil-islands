#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  This file is part of Cursed Earth.
#
#  Cursed Earth is an open source, cross-platform port of Evil Islands.
#  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

import darwin
import ppclinuxgnu
import x86linuxgnu
import x86linuxmingw
import x86windowsmingw

hosts = {
    "darwin": darwin,
    "ppc-linux-gnu": ppclinuxgnu,
    "x86-linux-gnu": x86linuxgnu,
    "x86-linux-mingw": x86linuxmingw,
    "x86-32-windows-mingw": x86windowsmingw,
    "x86-64-windows-mingw": x86windowsmingw,
}

# SCons PLATFORM -> HOST
defaults = {
    "darwin": "darwin",
    "posix": "x86-linux-gnu",
    "win32": "x86-64-windows-mingw",
}
