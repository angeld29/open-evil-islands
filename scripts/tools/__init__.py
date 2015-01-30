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

MODULE, TUNE = range(2)

def generate_codec(module, codecs, codec, env):
    codec = env.subst(codec)

    if "auto" != codec:
        if not codecs[codec][MODULE].exists(env):
            utils.interrupt("%s: codec '%s' not found", module, codec)
    else:
        for key, value in codecs.iteritems():
            codec = key
            if value[MODULE].exists(env):
                break
            logging.warning("%s: codec '%s' not found", module, codec)

    if not codecs[codec][MODULE].exists(env):
        utils.interrupt("%s: no appropriate codec found", module)

    logging.info("%s: using '%s' codec", module, codec)

    codecs[codec][MODULE].generate(env)
    codecs[codec][TUNE](env)

    return codec

def codec_exists(codecs, env):
    return any(value[MODULE].exists(env) for value in codecs.itervalues())
