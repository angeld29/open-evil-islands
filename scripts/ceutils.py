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

import os.path

import SCons.Errors

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
		def select_by_key(key, priority):
			node = next((node for node in values if get_key(node) == key), None)
			return (node, priority) if node else None
		result = select_by_key(env["PLATFORM"], 0) or \
				select_by_key(env["COMPILER"], 1) or \
				select_by_key(env["GRAPHLIB"], 2) or \
				select_by_key("generic", 3)
		if result:
			node, priority = result
			print "'{0}': best match '{1}', priority {2} ({3}), " \
					"selected from '{4}'".format(name, node.name,
					priority, ("platform", "compiler",
					"graphics library", "generic")[priority],
					", ".join(node.name for node in values))
			values.remove(node)
			for node in values:
				nodes.remove(node)
		else:
			raise SCons.Errors.StopError(
				"Can't deduct platform-depended "
				"file for '{0}', searched in '{1}'".format(name,
				", ".join(node.name for node in values)))
	return nodes
