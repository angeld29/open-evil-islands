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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celogging.h"
#include "cestring.h"
#include "ceroot.h"

static struct {
	bool inited;
	ce_texmng* texmng;
	ce_mprmng* mprmng;
} ce_root;

static bool ce_root_init_impl(const char* base_path)
{
	char path[512];

	ce_logging_write("root: loading with base path: '%s'...", base_path);

	if (NULL == (ce_root.texmng = ce_texmng_new())) {
		return false;
	}

	const char* texture_resources[] = { "textures", "redress", "menus" };
	for (int i = 0, n = sizeof(texture_resources) /
						sizeof(texture_resources[0]); i < n; ++i) {
		snprintf(path, sizeof(path), "%s/Res/%s.res",
				base_path, texture_resources[i]);
		if (!ce_texmng_register_resource(ce_root.texmng, path)) {
			return false;
		}
	}

	snprintf(path, sizeof(path), "%s/Maps", base_path);
	if (NULL == (ce_root.mprmng = ce_mprmng_new(path))) {
		return false;
	}

	return true;
}

bool ce_root_init(const char* base_path)
{
	assert(!ce_root.inited && "The root subsystem has already been inited");
	ce_root.inited = true;

	if (!ce_root_init_impl(base_path)) {
		ce_root_term();
		return false;
	}

	return true;
}

void ce_root_term(void)
{
	assert(ce_root.inited && "The root subsystem has not yet been inited");
	ce_root.inited = false;

	ce_mprmng_del(ce_root.mprmng);
	ce_texmng_del(ce_root.texmng);

	memset(&ce_root, 0, sizeof(ce_root));
}

ce_texmng* ce_root_get_texmng(void)
{
	assert(ce_root.inited && "The root subsystem has not yet been inited");
	return ce_root.texmng;
}

ce_mprmng* ce_root_get_mprmng(void)
{
	assert(ce_root.inited && "The root subsystem has not yet been inited");
	return ce_root.mprmng;
}
