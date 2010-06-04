/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include "celib.h"
#include "cealloc.h"
#include "cesoundmanager.h"

ce_soundmanager* ce_soundmanager_new(void)
{
	ce_soundmanager* soundmanager = ce_alloc(sizeof(ce_soundmanager));
	soundmanager->soundinstances = ce_vector_new();
	return soundmanager;
}

void ce_soundmanager_del(ce_soundmanager* soundmanager)
{
	if (NULL != soundmanager) {
		ce_vector_for_each(soundmanager->soundinstances, ce_soundinstance_del);
		ce_vector_del(soundmanager->soundinstances);
		ce_free(soundmanager, sizeof(ce_soundmanager));
	}
}

void ce_soundmanager_advance(ce_soundmanager* soundmanager, float elapsed)
{
	//ce_unused(elapsed);

	//for (size_t i = 0; i < soundmanager->soundinstances->count; ++i) {
	//	ce_soundinstance* instance = soundmanager->instances->items[i];
	//	ce_vector_remove_unordered(soundmanager->instances, i--);
	//}
}

void ce_soundmanager_play(ce_soundmanager* soundmanager, const char* path)
{
	//ce_soundinstance* soundinstance = ce_soundinstance_create_path(path);
	//if (NULL != soundinstance) {
	//	ce_vector_push_back(soundmanager->soundinstances, soundinstance);
	//	ce_soundinstance_play(soundinstance);
	//}
}
