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

from __future__ import with_statement

import hashlib
import os.path

from datetime import datetime
from itertools import chain

import SCons.Defaults
import SCons.Util
import SCons.Tool
import SCons.Node.FS

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

def make_relpath(node, env):
	relpath = os.path.relpath(node.get_abspath(), env.subst("$RCSTARTPATH"))
	return os.path.normpath(relpath)

def get_paths(node):
	return [os.path.normpath(line) for line in node.get_contents().splitlines()
									if len(line) > 0 and not line.startswith(';')]

def emit_rc(target, source, env):
	srcname = os.path.splitext(source[0].name)[0]
	tgtname = SCons.Util.adjustixes(srcname, "ce", "data")
	excludes = get_paths(source[0])
	env["RCSTARTPATH"] = env.Dir("$RCROOTPATH").get_abspath()
	return [os.path.join("$RCOBJPATH", SCons.Util.adjustixes(tgtname,
		env.subst("$OBJPREFIX"), env.subst("$OBJSUFFIX")))], [
		env.RcSource(os.path.join("$RCGENPATH", "src", tgtname),
		[node for node in get_nodes(["$RCROOTPATH"], ["*"], env)
				if not make_relpath(node, env) in excludes])]

rc_header = \
"""/*
 *  Resource data
 *
 *  Created: %s
 *       by: Cursed Earth build system
 *
 *  WARNING! All changes made in this file will be lost!
*/
"""

def build_rc_source(target, source, env):
	sizes, names = [], []
	with open(target[0].get_abspath(), "wt") as file:
		file.write(rc_header % datetime.now().strftime("%d %b %Y %H:%M:%S"))
		file.write("\n#include <stddef.h>\n")
		for node in source:
			name = hashlib.md5(make_relpath(node, env)).hexdigest()
			names.append(name)
			file.write("\nstatic const unsigned char ce_resource_data_%s[] = {\n" % name)
			contents = node.get_contents()
			sizes.append(len(contents))
			while len(contents) > 0:
				line, contents = contents[:15], contents[15:]
				file.write(",".join(hex(ord(ch)) for ch in line) + ",\n")
			file.write("};\n")
		file.write("\nconst size_t CE_RESOURCE_DATA_COUNT = %d;\n" % len(source))
		file.write("\nconst size_t ce_resource_data_sizes[] = {\n")
		for size in sizes:
			file.write("\t%d,\n" % size)
		file.write("};\n")
		file.write("\nconst char* ce_resource_data_paths[] = {\n")
		for node in source:
			file.write("\t\"%s\",\n" % make_relpath(node, env))
		file.write("};\n")
		file.write("\nconst unsigned char* ce_resource_data[] = {\n")
		for name in names:
			file.write("\tce_resource_data_%s,\n" % name)
		file.write("};\n")

def generate(env):
	env.SetDefault(
		RCROOTPATH="",
		RCGENPATH="",
		RCOBJPATH="",
		RCSTARTPATH="",

		RCSUFFIX=".cerc",

		RCSOURCEPREFIX="",
		RCSOURCESUFFIX=".c",
		RCSOURCESRCSUFFIX="",
	)

	env.Append(
		BUILDERS={
			"RcSource": SCons.Builder.Builder(
				action=build_rc_source,
				prefix="$RCSOURCEPREFIX",
				suffix="$RCSOURCESUFFIX",
				src_suffix="$RCSOURCESRCSUFFIX",
			),
		},
	)

	objbuilder, _ = SCons.Tool.createObjBuilders(env)

	objbuilder.add_action("$RCSUFFIX", SCons.Defaults.CAction)
	objbuilder.add_emitter("$RCSUFFIX", emit_rc)

def exists(env):
	return True
