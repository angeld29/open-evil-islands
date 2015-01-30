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

import utils
import compilers.gnuc
import graphiclibs.opengl
import platforms.posix

def configure(env):
    env["CE_LINUX_BIT"] = True

    if env["PLATFORM"] != "posix": # TODO: SCons PLATFORM variable is weak
        utils.interrupt("%s: this host is available only on Linux", env["HOST"])

    logging.info("%s: using Linux with GNU C/C++ x86 compiler", env["HOST"])

    platforms.posix.configure(env)
    compilers.gnuc.configure(env)
    graphiclibs.opengl.configure(env)
