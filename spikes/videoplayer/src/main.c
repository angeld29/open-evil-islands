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
#include "ceopengl.h"
#include "cetexture.h"
#include "ceroot.h"
#include "cesoundinstance.h"
#include "cevideoinstance.h"

static ce_optparse* optparse;

static ce_soundinstance* soundinstance;
static ce_videoinstance* videoinstance;

static ce_mmpfile* rgba;
static ce_texture* texture;

static ce_inputsupply* inputsupply;
static ce_inputevent* pause_event;

static bool pause;

static void clean()
{
	ce_inputsupply_del(inputsupply);

	ce_texture_del(texture);
	ce_mmpfile_del(rgba);

	ce_videoinstance_del(videoinstance);
	ce_soundinstance_del(soundinstance);

	ce_optparse_del(optparse);
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_inputsupply_advance(inputsupply, elapsed);

	if (pause_event->triggered) {
		pause = !pause;
		if (NULL != soundinstance) {
			if (pause) ce_soundinstance_pause(soundinstance);
			else ce_soundinstance_play(soundinstance);
		}
	}

	if (!pause) {
		if (NULL != soundinstance) {
			ce_videoinstance_sync(videoinstance, ce_soundinstance_time(soundinstance));
		} else {
			ce_videoinstance_advance(videoinstance, elapsed);
		}
	}

	ce_mmpfile* ycbcr = ce_videoinstance_acquire_frame(videoinstance);
	if (NULL != ycbcr) {
		if (NULL == rgba) {
			rgba = ce_mmpfile_new(ycbcr->width, ycbcr->height, 1,
									CE_MMPFILE_FORMAT_R8G8B8A8, 0);
		}
		// TODO: shader
		ce_mmpfile_convert2(ycbcr, rgba);
		if (NULL != texture) {
			ce_texture_replace(texture, rgba);
		} else {
			texture = ce_texture_new("frame", rgba);
		}
		ce_videoinstance_release_frame(videoinstance);
	}
}

static void render(void* listener)
{
	ce_unused(listener);

	if (NULL == texture) {
		return;
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	ce_texture_bind(texture);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, texture->width, 0, texture->height);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(0, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(texture->width, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(texture->width, texture->height);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(0, texture->height);

	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	ce_texture_unbind(texture);
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

static ce_videoinstance* create_videoinstance(const char* path)
{
	ce_memfile* memfile = ce_memfile_open_path(path);
	if (NULL == memfile) {
		return NULL;
	}

	ce_videoresource* videoresource = ce_videoresource_new(memfile);
	if (NULL == videoresource) {
		ce_memfile_close(memfile);
		return NULL;
	}

	ce_videoinstance* videoinstance = ce_videoinstance_new(videoresource);
	if (NULL == videoinstance) {
		ce_videoresource_del(videoresource);
		return NULL;
	}

	return videoinstance;
}

int main(int argc, char* argv[])
{
	ce_alloc_init();

	optparse = ce_root_create_optparse();

	ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any *.ogv file in 'CE/Movies'");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		clean();
		return EXIT_FAILURE;
	}

	atexit(clean);

	const char* track;
	ce_optparse_get(optparse, "track", &track);

	soundinstance = create_soundinstance(track);
	videoinstance = create_videoinstance(track);

	if (NULL == videoinstance) {
		ce_logging_fatal("main: could not play video track '%s'", track);
		return EXIT_FAILURE;
	}

	if (NULL != soundinstance) {
		ce_soundinstance_play(soundinstance);
	} else {
		ce_logging_info("main: no audio track found '%s'", track);
	}

	ce_scenemng_listener scenemng_listener = {.advance = advance, .render = render};
	ce_scenemng_add_listener(ce_root.scenemng, &scenemng_listener);

	inputsupply = ce_inputsupply_new(ce_root.renderwindow->inputcontext);
	pause_event = ce_inputsupply_single_front(inputsupply,
					ce_inputsupply_button(inputsupply, CE_KB_SPACE));

	return ce_root_exec();
}
