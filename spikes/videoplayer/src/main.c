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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "cesoundinstance.h"

static ce_optparse* optparse;

static ce_input_supply* input_supply;
static ce_input_event* reset_event;

static void clean()
{
	ce_input_supply_del(input_supply);
	ce_optparse_del(optparse);
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_input_supply_advance(input_supply, elapsed);

	if (reset_event->triggered) {
	}
}

int main(int argc, char* argv[])
{
	ce_alloc_init();
	atexit(clean);

	optparse = ce_root_create_optparse();

	ce_optparse_add(optparse, "track1", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any *.oga file in 'CE/Stream'");
	ce_optparse_add(optparse, "track2", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any *.oga file in 'CE/Stream'");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		return EXIT_FAILURE;
	}

	const char* track;
	ce_optparse_get(optparse, "track", &track);

	ce_scenemng_listener scenemng_listener = {.advance = advance, .render = NULL};
	ce_scenemng_add_listener(ce_root.scenemng, &scenemng_listener);

	input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
	reset_event = ce_input_supply_single_front(input_supply,
					ce_input_supply_button(input_supply, CE_KB_R));

	int code = ce_root_exec();

	return code;
}
