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
#include "cehudfps.h"

struct ce_hudfps {
	int frame_count;
	float time;
	void* font;
	char text[32];
};

ce_hudfps* ce_hudfps_new(void)
{
	ce_hudfps* fps = ce_alloc(sizeof(ce_hudfps));
	if (NULL == fps) {
		ce_logging_error("hudfps: could not allocate memory");
		return NULL;
	}
	fps->frame_count = 0;
	fps->time = 0.0f;
	fps->font = GLUT_BITMAP_HELVETICA_18;
	ce_strlcpy(fps->text, "FPS: updating...", sizeof(fps->text));
	return fps;
}

void ce_hudfps_del(ce_hudfps* fps)
{
	ce_free(fps, sizeof(ce_hudfps));
}

void ce_hudfps_advance(ce_hudfps* fps, float elapsed)
{
	++fps->frame_count;
	if ((fps->time += elapsed) >= 1.0f) {
		snprintf(fps->text, sizeof(fps->text), "FPS: %d", fps->frame_count);
		fps->frame_count = 0;
		fps->time = 0.0f;
	}
}

void ce_hudfps_render(ce_hudfps* fps)
{
	static float color[3] = { 1.0f, 0.0f, 0.0f };
	glColor3fv(color);
	glRasterPos2i(0, 0);
	const char* str = fps->text;
	while (*str) {
		glutBitmapCharacter(fps->font, *str++);
	}
}
