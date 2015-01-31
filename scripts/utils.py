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

import os
import logging
import itertools

import SCons

def interrupt(message, *args, **kwargs):
    logging.critical(message, *args, **kwargs)
    raise SCons.Errors.StopError("A critical error has occurred and SCons can not continue building.")

def traverse_files(node, patterns, env):
    return (file for pattern in patterns for file in node.glob(pattern) if isinstance(file, SCons.Node.FS.File))

def traverse_dirs(node, patterns, env):
    return (file for dir in node.glob("*") if isinstance(dir, SCons.Node.FS.Dir) for file in traverse_all(dir, patterns, env))

def traverse_all(node, patterns, env):
    return itertools.chain(traverse_files(node, patterns, env), traverse_dirs(node, patterns, env))

def get_nodes(paths, patterns, env):
    return (file for path in paths for file in traverse_all(env.Dir(path), patterns, env))

bit_suffix_map = [
    ("CE_LINUX_BIT", "linux"),
    ("CE_POSIX_BIT", "posix"),
    ("CE_WINDOWS_BIT", "windows"),
    ("CE_X11_BIT", "x11"),
    ("CE_OPENGL_BIT", "opengl"),
    ("CE_GNUC_BIT", "gnuc"),
    ("CE_MINGW_BIT", "mingw"),
    ("CE_GCC_BIT", "gcc"),
    ("CE_GENERIC_BIT", "generic"),
]

def filter_sources(env, nodes):
    nodes = nodes[:]
    get_name = lambda node: os.path.splitext(node.name)[0].split('_')[0]
    get_key = lambda node: os.path.splitext(node.name)[0].split('_')[1]
    key_nodes = (node for node in nodes if -1 != node.name.find('_'))
    cache = dict()
    for node in key_nodes:
        values = cache.get(get_name(node), [])
        values.append(node)
        cache[get_name(node)] = values
    for name, values in cache.iteritems():
        def select():
            for suffix in (suffix for bit, suffix in bit_suffix_map if env.has_key(bit) and env[bit] is True):
                for node in (node for node in values if get_key(node) == suffix):
                    return node
            return None
        node = select()
        if node:
            logging.debug("utils: using '%s' for '%s' from [%s]", node.name, name, ", ".join(node.name for node in values))
            values.remove(node)
        else:
            logging.debug("utils: could not deduct platform-depended file for '%s' from [%s]", name, ", ".join(node.name for node in values))
        for node in values:
            nodes.remove(node)
    return nodes
