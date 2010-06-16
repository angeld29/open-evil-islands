/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>

#include "cealloc.h"
#include "cesoundmixer.h"

ce_sound_mixer* ce_sound_mixer_new(void)
{
	ce_sound_mixer* sound_mixer = ce_alloc(sizeof(ce_sound_mixer));
	return sound_mixer;
}

void ce_sound_mixer_del(ce_sound_mixer* sound_mixer)
{
	if (NULL != sound_mixer) {
		ce_free(sound_mixer, sizeof(ce_sound_mixer));
	}
}
