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
import hashlib
import datetime

import SCons

import utils

def fix_path(path):
    return path.lower().replace('\\', '/')

def make_relpath(node, env):
    return fix_path(os.path.relpath(node.get_abspath(), env.subst("$RCROOTABSPATH")))

def get_paths(node):
    return [fix_path(line) for line in node.get_contents().splitlines() if len(line) > 0 and not line.startswith(';')]

def make_nodes(cache, env):
    return [env.File(os.path.join("$RCROOTABSPATH", os.path.normpath(path))) for path in get_paths(cache)]

def write_header(file, header):
    file.write(header % datetime.datetime.now().strftime("%d %b %Y %H:%M:%S"))

cache_header = \
""";  Resource cache
;
;  Created: %s
;       by: Cursed Earth build system
;
;  WARNING! All changes made in this file will be lost!
"""

def emit_rc(target, source, env):
    srcname = os.path.splitext(source[0].name)[0]
    tgtname = SCons.Util.adjustixes(srcname, "", "_data")
    excludes = get_paths(source[0])

    # save absolute resource root path especially for builder because
    # last one will be invoked from the project root directory
    env["RCROOTABSPATH"] = env.Dir("$RCROOTPATH").get_abspath()

    # cache is needed... for example, SCons will not be able to process
    # dependencies correctly if we only rename some files
    cache = env.File(os.path.join("$RCBUILDPATH", SCons.Util.adjustixes(srcname, "", env.subst("$RCSOURCESRCSUFFIX"))))

    nodes = [node for node in utils.get_nodes(["$RCROOTPATH"], ["*"], env) if not make_relpath(node, env) in excludes]
    paths = sorted(make_relpath(node, env) for node in nodes)

    if not env.GetOption("clean") and (not os.path.exists(cache.get_abspath()) or paths != get_paths(cache)):
        # do not create a cache file if scons -c
        # note that SCons will create directories only before the builders
        if not os.path.exists(env.Dir("$RCBUILDPATH").get_abspath()):
            env.Execute(SCons.Defaults.Mkdir("$RCBUILDPATH"))
        with open(cache.get_abspath(), "wt") as file:
            write_header(file, cache_header)
            file.write('\n' + '\n'.join(paths) + '\n')

    target[0] = os.path.join("$RCBUILDOBJPATH", SCons.Util.adjustixes(tgtname, env.subst("$OBJPREFIX"), env.subst("$OBJSUFFIX")))
    source[0] = env.RcSource(os.path.join("$RCBUILDPATH", "src", tgtname), cache)

    # cache node is not a normal target, so mark it as cleanable
    env.Clean(target[0], cache)

    return target, source

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

def build_rc_source(target, source, env):
    sizes, names = [], []
    nodes = make_nodes(source[0], env)
    with open(target[0].get_abspath(), "wt") as file:
        write_header(file, c_header)
        file.write("\n#include \"ceresourcedata.h\"\n")
        for node in nodes:
            name = hashlib.md5(make_relpath(node, env)).hexdigest()
            names.append(name)
            file.write("\nstatic const unsigned char ce_resource_data_%s[] = {\n" % name)
            contents = node.get_contents()
            sizes.append(len(contents))
            while len(contents) > 0:
                line, contents = contents[:15], contents[15:]
                file.write('\t' + ','.join(hex(ord(ch)) for ch in line) + ",\n")
            file.write("};\n")
        file.write("\nconst size_t CE_RESOURCE_DATA_COUNT = %d;\n" % len(nodes))
        file.write("\nconst size_t ce_resource_data_sizes[] = {\n")
        for size in sizes:
            file.write("\t%d,\n" % size)
        file.write("};\n")
        file.write("\nconst char* ce_resource_data_paths[] = {\n")
        for node in nodes:
            file.write("\t\"%s\",\n" % make_relpath(node, env))
        file.write("};\n")
        file.write("\nconst unsigned char* ce_resource_data[] = {\n")
        for name in names:
            file.write("\tce_resource_data_%s,\n" % name)
        file.write("};\n")

def generate(env):
    env.SetDefault(
        RCROOTPATH="",
        RCROOTABSPATH="",
        RCBUILDPATH="",
        RCBUILDOBJPATH="",

        RCSUFFIX=".cerc",

        RCSOURCEPREFIX="",
        RCSOURCESUFFIX=".c",
        RCSOURCESRCSUFFIX=".cerccache",
    )

    env.Append(
        SCANNERS=SCons.Scanner.Scanner(
            function=lambda node, env, *args: make_nodes(node, env),
            skeys="$RCSOURCESRCSUFFIX",
            recursive=False,
        ),
        BUILDERS={
            "RcSource": SCons.Builder.Builder(
                action=build_rc_source,
                prefix="$RCSOURCEPREFIX",
                suffix="$RCSOURCESUFFIX",
                src_suffix="$RCSOURCESRCSUFFIX",
                single_source=True,
            ),
        },
    )

    objbuilder, _ = SCons.Tool.createObjBuilders(env)

    objbuilder.add_action("$RCSUFFIX", SCons.Defaults.CAction)
    objbuilder.add_emitter("$RCSUFFIX", emit_rc)

def exists(env):
    return True
