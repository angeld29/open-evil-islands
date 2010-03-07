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
#include "cefont.h"

struct ce_font {
	void* data;
	int height;
};

static void* ce_font_datas[CE_FONT_TYPE_COUNT] = {
	GLUT_BITMAP_HELVETICA_18
};

static int ce_font_heights[CE_FONT_TYPE_COUNT] = {
	18
};

ce_font* ce_font_new(ce_font_type type)
{
	ce_font* font = ce_alloc(sizeof(ce_font));
	if (NULL == font) {
		ce_logging_error("font: could not allocate memory");
		return NULL;
	}

	font->data = ce_font_datas[type];
	font->height = ce_font_heights[type];

	return font;
}

void ce_font_del(ce_font* font)
{
	ce_free(font, sizeof(ce_font));
}

int ce_font_get_height(ce_font* font)
{
	return font->height;
}

int ce_font_get_width(ce_font* font, const char* text)
{
	return glutBitmapLength(font->data, (const unsigned char*)text);
}

void ce_font_render(ce_font* font, int x, int y,
					const ce_color* color, const char* text)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	gluOrtho2D(viewport[0], viewport[2], viewport[1], viewport[3]);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor4f(color->r, color->g, color->b, color->a);
	glRasterPos2i(x, y);

	while (*text) {
		glutBitmapCharacter(font->data, *text++);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
