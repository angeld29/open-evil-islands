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

import cetools
import ceoggenc

def tune_oggenc(env):
    env.Replace(
        OGGENCFLAGS="--quality=$WAV2OGAQUALITY",
        OGGENCSOURCE="$WAV2OGASOURCE",

        WAV2OGACOM="$OGGENCCOM",
        WAV2OGACOMSTR="$OGGENCCOMSTR",
    )

    env["BUILDERS"]["Wav2Oga"] = env["BUILDERS"]["OggEnc"]

codecs = {
    ceoggenc.UTILITY: (ceoggenc, tune_oggenc),
}

def generate(env):
    env.SetDefault(
        WAV2OGACODEC="auto",
        WAV2OGAQUALITY="3",
        WAV2OGASOURCE="$SOURCE",
    )

    cetools.generate_codec("wav2oga", codecs, "$WAV2OGACODEC", env)

def exists(env):
    return cetools.codec_exists(codecs, env)
