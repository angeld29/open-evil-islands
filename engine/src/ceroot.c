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

#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

struct ce_root ce_root;

bool ce_root_init(const char* ei_path)
{
	assert(!ce_root.inited && "the root subsystem has already been inited");

	ce_alloc_init();
	ce_logging_init();

	ce_root.renderwindow = ce_renderwindow_new();
	ce_root.rendersystem = ce_rendersystem_new();
	ce_root.scenemng = ce_scenemng_new(ei_path);

	ce_input_init();

	return ce_root.inited = true;
}

void ce_root_term(void)
{
	assert(ce_root.inited && "the root subsystem has not yet been inited");
	ce_root.inited = false;

	ce_input_term();

	ce_scenemng_del(ce_root.scenemng), ce_root.scenemng = NULL;
	ce_rendersystem_del(ce_root.rendersystem), ce_root.rendersystem = NULL;
	ce_renderwindow_del(ce_root.renderwindow), ce_root.renderwindow = NULL;

	ce_logging_term();
	ce_alloc_term();
}

void ce_root_exec(void)
{
	assert(ce_root.inited && "the root subsystem has not yet been inited");

	while (ce_renderwindow_pump(ce_root.renderwindow)) {
		ce_scenemng_advance(ce_root.scenemng);
		ce_scenemng_render(ce_root.scenemng);
		ce_renderwindow_swap(ce_root.renderwindow);
	}
}
