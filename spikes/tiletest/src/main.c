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
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <GL/glut.h>

#ifdef _WIN32
// fu... win32
#undef near
#undef far
#endif

#include "cegl.h"
#include "celogging.h"
#include "cealloc.h"
#include "ceinput.h"
#include "cetimer.h"
#include "cemath.h"
#include "cevec3.h"
#include "cescenemng.h"
#include "cecfgfile.h"
#include "celightcfg.h"
#include "ceoptparse.h"
#include "cemmphlp.h"
#include "cemprhlp.h"

static void rotate90_rgba8(int width, int height,
								void* restrict dst,
								const void* restrict src)
{
	uint32_t* d = dst;
	const uint32_t* s = src;
	for (int i = 0; i < width; ++i) {
		for (int j = height - 1; j >= 0; --j) {
			*d++ = s[j * width + i];
		}
	}
}

static void rotate180_rgba8(int width, int height, void* texels)
{
	uint32_t tmp;
	uint32_t* head = texels;
	uint32_t* tail = head + width * height - 1;
	while (head < tail) {
		tmp = *head;
		*head++ = *tail;
		*tail-- = tmp;
	}
}

static void rotate270_rgba8(int width, int height,
								void* restrict dst,
								const void* restrict src)
{
	uint32_t* d = dst;
	const uint32_t* s = src;
	for (int i = width - 1; i >= 0; --i) {
		for (int j = 0; j < height; ++j) {
			*d++ = s[j * width + i];
		}
	}
}

int main(int argc, char* argv[])
{
	ce_logging_init();
#ifdef NDEBUG
	ce_logging_set_level(CE_LOGGING_LEVEL_WARNING);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif
	ce_alloc_init();

	ce_optparse* optparse = ce_optparse_new(0, 0, 0, "Test");
	ce_optgroup* general = ce_optparse_create_group(optparse, "general");
	ce_optoption* ei_path = ce_optgroup_create_option(general,
		"ei_path", 'b', "ei-path", CE_OPTACTION_STORE,
		"path to EI root dir (current dir by default)", ".");

	if (!ce_optparse_parse_args(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	if (!glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(400, 300);
		glutCreateWindow("Cursed Earth: MPR Viewer");
	}

	ce_input_init();
	ce_gl_init();

	ce_scenemng* scenemng = ce_scenemng_new(ei_path->value->str);
	if (NULL == scenemng) {
		return EXIT_FAILURE;
	}

	ce_optparse_del(optparse);

	ce_mprfile* mprfile = ce_mprmng_open_mprfile(scenemng->mprmng, "bz8k");
	ce_mmpfile* mmpfile = ce_texmng_open_mmpfile(scenemng->texmng, "bz8k000");
	ce_mmphlp_dxt_decompress_rgba8(mmpfile);

	void* data = ce_alloc(mmpfile->size);
	rotate270_rgba8(mmpfile->width, mmpfile->height, data, mmpfile->texels);
	ce_free(mmpfile->data, mmpfile->size);
	mmpfile->texels = data;
	mmpfile->data = data;

	ce_mmphlp_rgba8_compress_dxt(mmpfile, CE_MMPFILE_FORMAT_DXT1);
	ce_texmng_save_mmpfile(scenemng->texmng, "bz8k000000", mmpfile);
	ce_mmpfile_close(mmpfile);
	ce_mprfile_close(mprfile);

	ce_scenemng_del(scenemng);
	ce_gl_term();
	ce_input_term();
	ce_alloc_term();
	ce_logging_term();

	return EXIT_SUCCESS;
}
