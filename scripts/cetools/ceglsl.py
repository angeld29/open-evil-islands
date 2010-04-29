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
from datetime import datetime

import SCons.Defaults
import SCons.Util
import SCons.Tool

def scan_shader(node, env, *args):
	src_path = os.path.dirname(node.srcnode().get_abspath())
	return env.Glob(os.path.join(src_path, "*.vert")) + \
			env.Glob(os.path.join(src_path, "*.frag"))

def emit_shader_source(target, source, env):
	name = os.path.splitext(source[0].name)[0] + "_" + source[0].suffix[1:]
	return [os.path.join(os.path.dirname(target[0].get_abspath()),
			SCons.Util.adjustixes(name, env.subst("$OBJPREFIX"),
										env.subst("$OBJSUFFIX")))], \
		[env.Shader(os.path.join("$GLSL_BUILDPATH", "src", name), source[0])]

def emit_shader_include(target, source, env):
	name = os.path.splitext(source[0].name)[0]
	env.Glsl(os.path.join("$GLSL_BUILDPATH", "include", name), source[0])
	return [], []

header = \
"""/*
 *  Shader binary source code
 *
 *  Created: %s
 *       by: Cursed Earth build system
 *
 *  WARNING! All changes made in this file will be lost!
*/

"""

def write_header(file):
	file.write(header % datetime.now().strftime("%d %b %Y %H:%M:%S"))

def build_shader_include(target, source, env):
	with open(target[0].get_abspath(), "wt") as file:
		write_header(file)
		sources = scan_shader(source[0], env)
		for node in sources:
			name = node.name.replace(".", "_")[2:] # also remove ce prefix
			file.write("extern const char ce_shader_data_%s[];\n" % name)
		file.write("\nstatic const int CE_SHADER_DATA_COUNT = %d;\n" % len(sources))
		file.write("static const char* ce_shader_data[][2] = {\n")
		for node in sources:
			name = node.name.replace(".", "_")[2:] # also remove ce prefix
			file.write("{ \"%s\", ce_shader_data_%s },\n" % (name, name))
		file.write("};\n")

def build_shader_source(target, source, env):
	name = os.path.splitext(target[0].name)[0][2:] # also remove ce prefix
	with open(target[0].get_abspath(), "wt") as file:
		write_header(file)
		file.write("const char ce_shader_data_%s[] = {\n" % name)
		contents = source[0].get_contents()
		while len(contents) > 0:
			line, contents = contents[:15], contents[15:]
			file.write(",".join(hex(ord(ch)) for ch in line) + ",\n")
		file.write("};\n")

def generate(env):
	env.SetDefault(
		GLSL_BUILDPATH="",

		GLSL_PREFIX="",
		GLSL_SUFFIX=".h",
		GLSL_SRCSUFFIX=".glsl",

		SHADER_PREFIX="",
		SHADER_SUFFIX=".c",
		SHADER_SRCSUFFIX=[".vert", ".frag"],
	)

	env.Append(
		BUILDERS={
			"Glsl": SCons.Builder.Builder(
				action=build_shader_include,
				prefix="$GLSL_PREFIX",
				suffix="$GLSL_SUFFIX",
				src_suffix="$GLSL_SRCSUFFIX",
				single_source=True,
			),
			"Shader": SCons.Builder.Builder(
				action=build_shader_source,
				prefix="$SHADER_PREFIX",
				suffix="$SHADER_SUFFIX",
				src_suffix="$SHADER_SRCSUFFIX",
				single_source=True,
			),
		},
		SCANNERS=SCons.Scanner.Scanner(
			function=scan_shader,
			skeys=[env.subst("$GLSL_SRCSUFFIX")],
			recursive=False,
		),
	)

	obj_builder, _ = SCons.Tool.createObjBuilders(env)

	for src_suffix in env["SHADER_SRCSUFFIX"]:
		obj_builder.add_action(src_suffix, SCons.Defaults.CAction)
		obj_builder.add_emitter(src_suffix, emit_shader_source)

	obj_builder.add_action("$GLSL_SRCSUFFIX", SCons.Defaults.CAction)
	obj_builder.add_emitter("$GLSL_SRCSUFFIX", emit_shader_include)

def exists(env):
	return True
