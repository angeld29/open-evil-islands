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

static ce_soundinstance* soundinstance1;
static ce_soundinstance* soundinstance2;

static ce_inputsupply* inputsupply;
static ce_inputevent* reset_event;

static void clean()
{
	ce_inputsupply_del(inputsupply);

	ce_soundinstance_del(soundinstance2);
	ce_soundinstance_del(soundinstance1);

	ce_optparse_del(optparse);
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_inputsupply_advance(inputsupply, elapsed);

	if (reset_event->triggered) {
		ce_soundinstance_stop(soundinstance2);
		ce_soundinstance_play(soundinstance2);
	}
}

static ce_soundinstance* create_soundinstance(const char* path)
{
	ce_memfile* memfile = ce_memfile_open_path(path);
	if (NULL == memfile) {
		return NULL;
	}

	ce_soundresource* soundresource = ce_soundresource_new_builtin(memfile);
	if (NULL == soundresource) {
		ce_memfile_close(memfile);
		return NULL;
	}

	ce_soundinstance* soundinstance = ce_soundinstance_new(soundresource);
	if (NULL == soundinstance) {
		ce_soundresource_del(soundresource);
		return NULL;
	}

	return soundinstance;
}

int main(int argc, char* argv[])
{
	ce_alloc_init();

	optparse = ce_root_create_optparse();

	//ce_optparse_add(optparse, "track1", CE_TYPE_STRING, NULL, true,
	//	NULL, NULL, "any *.oga file in 'CE/Stream'");
	ce_optparse_add(optparse, "track2", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any *.oga file in 'CE/Stream'");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		clean();
		return EXIT_FAILURE;
	}

	atexit(clean);

	const char *track1, *track2;
	ce_optparse_get(optparse, "track1", &track1);
	ce_optparse_get(optparse, "track2", &track2);

	soundinstance2 = create_soundinstance(track2);
	if (NULL == soundinstance2) {
		ce_logging_error("main: could not play '%s'", track2);
		return EXIT_FAILURE;
	}

	//ce_soundinstance_play(soundinstance1);
	ce_soundinstance_play(soundinstance2);

	ce_scenemng_listener scenemng_listener = {.advance = advance, .render = NULL};
	ce_scenemng_add_listener(ce_root.scenemng, &scenemng_listener);

	inputsupply = ce_inputsupply_new(ce_root.renderwindow->inputcontext);
	reset_event = ce_inputsupply_single_front(inputsupply,
					ce_inputsupply_button(inputsupply, CE_KB_R));

	return ce_root_exec();
}
