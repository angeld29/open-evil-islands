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

#include <GL/glut.h>

#include "cestr.h"
#include "celogging.h"
#include "cealloc.h"
#include "cefps.h"

struct ce_fps {
	int frame_count;
	float time;
	void* font;
	int font_height;
	char text[32];
};

ce_fps* ce_fps_new(void)
{
	ce_fps* fps = ce_alloc(sizeof(ce_fps));
	if (NULL == fps) {
		ce_logging_error("fps: could not allocate memory");
		return NULL;
	}
	fps->frame_count = 0;
	fps->time = 0.0f;
	fps->font = GLUT_BITMAP_HELVETICA_18;
	fps->font_height = 18;
	ce_strlcpy(fps->text, "FPS: updating...", sizeof(fps->text));
	return fps;
}

void ce_fps_del(ce_fps* fps)
{
	ce_free(fps, sizeof(ce_fps));
}

void ce_fps_advance(ce_fps* fps, float elapsed)
{
	++fps->frame_count;
	if ((fps->time += elapsed) >= 1.0f) {
		snprintf(fps->text, sizeof(fps->text), "FPS: %d", fps->frame_count);
		fps->frame_count = 0;
		fps->time = 0.0f;
	}
}

void ce_fps_render(ce_fps* fps)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int width = viewport[2];
	int height = viewport[3];

	gluOrtho2D(0.0f, width, 0.0f, height);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	static float color[3] = { 1.0f, 0.0f, 0.0f };
	glColor3fv(color);

	glRasterPos2i(width -
		glutBitmapLength(fps->font, (const unsigned char*)fps->text) - 10,
		height - fps->font_height - 10);

	for (const char* str = fps->text; *str; ++str) {
		glutBitmapCharacter(fps->font, *str);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
