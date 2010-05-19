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

import os.path
from datetime import datetime
from itertools import chain

import SCons.Defaults
import SCons.Util
import SCons.Tool
import SCons.Node.FS

traverse_files = lambda node, patterns, env: (file for pattern
						in patterns for file in node.glob(pattern)
						if isinstance(file, SCons.Node.FS.File))

traverse_dirs = lambda node, patterns, env: (file for dir
					in node.glob("*") if isinstance(dir, SCons.Node.FS.Dir)
					for file in traverse_all(dir, patterns, env))

traverse_all = lambda node, patterns, env: chain(traverse_files(node,
					patterns, env), traverse_dirs(node, patterns, env))

get_nodes = lambda paths, patterns, env: (file for path in paths
				for file in traverse_all(env.Dir(path), patterns, env))

get_paths = lambda cache: (path for path in cache.get_contents().split('\n')
							if len(path) > 0 and not path.startswith(';'))

def make_path(node, start):
	return os.path.normpath(os.path.relpath(node.get_abspath(), start))

def write_header(file, header):
	file.write(header % datetime.now().strftime("%d %b %Y %H:%M:%S"))

cache_header = \
""";  Resource cache
;
;  Created: %s
;       by: Cursed Earth build system
;
;  WARNING! All changes made in this file will be lost!

"""

def emit_rc(target, source, env):
	cache = env.File(os.path.join("$RCBUILDPATH",
		SCons.Util.adjustixes("resource", "", env.subst("$RCSUFFIX"))))
	print 'cache:', cache.get_abspath()

	# it's not a normal target, so mark this node as cleanable
	env.Clean(cache, cache)

	resdir = os.path.dirname(source[0].get_abspath())

	paths = sorted(make_path(node, resdir)
					for node in get_nodes([resdir], ["*"], env)
					if not node.name.endswith(env.subst("$RCSRCSUFFIX")))
	print paths

	dfdf = asdasd

	src_path = os.path.dirname(source[0].srcnode().get_abspath())
	names = sorted(shader.name.lower() for shader in
					env.Glob(os.path.join(src_path, "*.vert")) +
					env.Glob(os.path.join(src_path, "*.frag")))

	if not os.path.exists(glsl.get_abspath()) or names != get_shader_names(glsl):
		with open(glsl.get_abspath(), "wt") as file:
			write_header(file, cache_header)
			file.write('\n'.join(names) + '\n')

	# only add a node (include file) to SCons tree
	# SCons will execute builder automatically if necessary
	env.ShaderInclude(os.path.join("$GLSL_BUILDPATH",
		"include", os.path.splitext(glsl.name)[0]), glsl)

	name = os.path.splitext(source[0].name)[0] + "_" + source[0].suffix[1:]
	return [os.path.join(os.path.dirname(target[0].get_abspath()),
			SCons.Util.adjustixes(name, env.subst("$OBJPREFIX"),
										env.subst("$OBJSUFFIX")))], \
		[env.ShaderSource(os.path.join("$GLSL_BUILDPATH", "src", name), [source[0]])]

c_header = \
"""/*
 *  Resource data
 *
 *  Created: %s
 *       by: Cursed Earth build system
 *
 *  WARNING! All changes made in this file will be lost!
*/

"""

def build_rc_include(target, source, env):
	with open(target[0].get_abspath(), "wt") as file:
		write_header(file, c_header)
		names = [name.rstrip("\r").replace(".", "_")[2:] # also remove ce prefix
				for name in get_shader_names(source[0])]
		for name in names:
			file.write("extern const char ce_shaderdata_%s[];\n" % name)
		file.write("\nstatic const int CE_SHADERDATA_COUNT = %d;\n" % len(names))
		file.write("static const char* ce_shaderdata[][2] = {\n")
		for name in names:
			file.write("{ \"%s\", ce_shaderdata_%s },\n" % (name, name))
		file.write("};\n")

def build_rc_source(target, source, env):
	name = os.path.splitext(target[0].name)[0][2:] # also remove ce prefix
	with open(target[0].get_abspath(), "wt") as file:
		write_header(file, c_header)
		file.write("const char ce_shaderdata_%s[] = {\n" % name)
		contents = source[0].get_contents() + '\0'
		while len(contents) > 0:
			line, contents = contents[:15], contents[15:]
			file.write(",".join(hex(ord(ch)) for ch in line) + ",\n")
		file.write("};\n")

def generate(env):
	env.SetDefault(
		RCBUILDPATH="",
		RCBUILDOBJPATH="",

		RCPREFIX="",
		RCSUFFIX=".cerccache",
		RCSRCSUFFIX=".cerc",

		RCINCLUDEPREFIX="",
		RCINCLUDESUFFIX=".h",
		RCINCLUDESRCSUFFIX="$RCSUFFIX",

		RCSOURCEPREFIX="",
		RCSOURCESUFFIX=".c",
		RCSOURCESRCSUFFIX="",
	)

	env.Append(
		BUILDERS={
			"RcInclude": SCons.Builder.Builder(
				action=build_rc_include,
				prefix="$RCINCLUDEPREFIX",
				suffix="$RCINCLUDESUFFIX",
				src_suffix="$RCINCLUDESRCSUFFIX",
				single_source=True,
			),
			"RcSource": SCons.Builder.Builder(
				action=build_rc_source,
				prefix="$RCSOURCEPREFIX",
				suffix="$RCSOURCESUFFIX",
				src_suffix="$RCSOURCESRCSUFFIX",
				single_source=True,
			),
		},
	)

	obj_builder, _ = SCons.Tool.createObjBuilders(env)

	obj_builder.add_action("$RCSRCSUFFIX", SCons.Defaults.CAction)
	obj_builder.add_emitter("$RCSRCSUFFIX", emit_rc)

def exists(env):
	return True
