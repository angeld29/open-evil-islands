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
import logging

from itertools import chain

import SCons

import ceerrors

def traverse_files(node, patterns, env):
	return (file for pattern in patterns
				for file in node.glob(pattern)
				if isinstance(file, SCons.Node.FS.File))

def traverse_dirs(node, patterns, env):
	return (file for dir in node.glob("*")
				if isinstance(dir, SCons.Node.FS.Dir)
				for file in traverse_all(dir, patterns, env))

def traverse_all(node, patterns, env):
	return chain(traverse_files(node, patterns, env),
				traverse_dirs(node, patterns, env))

def get_nodes(paths, patterns, env):
	return (file for path in paths
				for file in traverse_all(env.Dir(path), patterns, env))

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
			for node in (node for node in values if get_key(node) == key):
				return (node, priority)
			return None
		result = select_by_key(env["HOST"].replace("-", ""), 0) or \
				select_by_key(env["TARGET_PLATFORM"], 1) or \
				select_by_key(env["COMPILER"], 2) or \
				select_by_key(env["GRAPHICS_LIBRARY"], 3) or \
				select_by_key("generic", 4)
		if result:
			node, priority = result
			logging.debug("'%s': best match '%s', priority %d (%s), "
						"selected from '%s'", name, node.name,
						priority, ("host", "target platform", "compiler",
						"graphics library", "generic")[priority],
						", ".join(node.name for node in values))
			values.remove(node)
		else:
			logging.debug("could not deduct platform-depended file "
				"'%s' [%s]", name, ", ".join(node.name for node in values))
		for node in values:
			nodes.remove(node)
	return nodes

try:
	relpath = os.path.relpath
except AttributeError:
	def relpath(path, start=os.curdir):
		"""Return a relative version of a path"""

		if not path:
			raise ValueError("no path specified")

		start_list = os.path.abspath(start).split(os.sep)
		path_list = os.path.abspath(path).split(os.sep)

		if start_list[0].lower() != path_list[0].lower():
			unc_path, rest = os.path.splitunc(path)
			unc_start, rest = os.path.splitunc(start)
			if bool(unc_path) ^ bool(unc_start):
				raise ValueError("cannot mix UNC and non-UNC paths "
									"(%s and %s)" % (path, start))
			else:
				raise ValueError("path is on drive %s, start on drive %s" %
											(path_list[0], start_list[0]))

		# work out how much of the filepath is shared by start and path
		for i in range(min(len(start_list), len(path_list))):
			if start_list[i].lower() != path_list[i].lower():
				break
			else:
				i += 1

		rel_list = [os.pardir] * (len(start_list)-i) + path_list[i:]
		if not rel_list:
			return curdir

		return os.path.join(*rel_list)
