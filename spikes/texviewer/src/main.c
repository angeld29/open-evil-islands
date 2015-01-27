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
#include <limits.h>
#include <time.h>
#include <assert.h>

/*#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceinput.h"
#include "cetimer.h"
#include "ceresfile.h"
#include "cetexture.h"*/

// TODO: reanimation after engine 2.x

/*static ce_resfile* res;
static ce_texture* tex;
static ce_timer* tmr;

static bool rndmode = false;
static int delay;
static bool slideshow = true;
static int index;

static ce_input_event_supply* es;
static ce_input_event* next_texture_event;

static void next_texture(int unused);

static void advance_index()
{
    index = rndmode ? rand() % (int)res->node_count : index + 1;
}

static void idle(void)
{
    ce_timer_advance(tmr);

    float elapsed = ce_timer_elapsed(tmr);

    ce_input_advance(elapsed);

    ce_input_event_supply_advance(es, elapsed);

    if (ce_input_test(CE_KB_ESCAPE)) {
        ce_input_event_supply_del(es);
        ce_timer_del(tmr);
        ce_texture_del(tex);
        ce_resfile_close(res);
        ce_gl_term();
        ce_input_term();
        ce_alloc_term();
        ce_logging_term();
        exit(EXIT_SUCCESS);
    }

    if (ce_input_event_triggered(next_texture_event)) {
        if (!slideshow) {
            advance_index();
            glutTimerFunc(0, next_texture, 0);
        }
    }
}

static void display(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (NULL != tex) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ce_texture_bind(tex);
    } else {
        glColor3f(1.0f, 0.0f, 0.0f);
    }

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glEnd();

    if (NULL != tex) {
        ce_texture_unbind(tex);
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    glutSwapBuffers();
}

static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

static bool generate_texture(int index)
{
    ce_texture_del(tex);
    tex = NULL;

    // TODO: use texmng
    ce_mmpfile* mmpfile = ce_mmpfile_new_resfile(res, index);

    if (CE_MMPFILE_FORMAT_PNT3 == mmpfile->format) {
        ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_ARGB8);
    }

    tex = ce_texture_new("stub", mmpfile);
    ce_mmpfile_del(mmpfile);

    return NULL != tex;
}

static void next_texture(int unused)
{
    if (0 == res->node_count) {
        return;
    }

    if (0 > index || index >= (int)res->node_count) {
        ce_logging_write("main: all textures (%d) have been browsed, "
            "let's make a fresh start", res->node_count);
        index = 0;
    }

    int d = delay;

    if (!generate_texture(index)) {
        ce_logging_error("main: could not load texture: '%s'",
            ce_resfile_node_name(res, index));
        d = 0;
    } else {
        glutPostRedisplay();
    }

    if (slideshow) {
        advance_index();
        glutTimerFunc(d, next_texture, 0);
    }
}

static void usage(const char* progname, void* argtable[])
{
    fprintf(stderr, "Cursed Earth is an open source, "
        "cross-platform port of Evil Islands\n"
        "Copyright (C) 2009-2010 Yanis Kurganov\n\n");

    fprintf(stderr, "This program is part of Cursed Earth spikes\n"
        "Texture Viewer - view Evil Islands textures\n\n");

    fprintf(stderr, "usage: %s", progname);
    arg_print_syntax(stderr, argtable, "\n");
    arg_print_glossary_gnu(stderr, argtable);

    void* ctrtable[] = {
        arg_rem("n", "next texture (manual mode)"),
        arg_end(0)
    };

    fprintf(stderr, "controls:\n");
    arg_print_glossary_gnu(stderr, ctrtable);
    arg_freetable(ctrtable, sizeof(ctrtable) / sizeof(ctrtable[0]));
}*/

int main(int CE_UNUSED(argc), char** CE_UNUSED(argv))
{
    /*struct arg_lit* help = arg_lit0("h", "help", "display this help and exit");
    struct arg_lit* version = arg_lit0("v", "version",
        "display version information and exit");
    struct arg_lit* random = arg_lit0("r", "random",
        "show textures in random mode");
    struct arg_int* dly = arg_int0("d", "delay", "MSEC",
        "specify slideshow delay (500 by default)");
    struct arg_lit* manual = arg_lit0("m", "manual",
        "manual mode (slideshow will be disabled)");
    struct arg_str* name = arg_str0("n", "name", "NAME",
        "specify texture name (slideshow will be disabled)");
    struct arg_int* idx = arg_int0("i", "index", "INDEX",
        "specify texture index (slideshow will be disabled)");
    struct arg_str* file = arg_str1(NULL, NULL,
        "FILE", "path to res file with textures");
    struct arg_end* end = arg_end(3);

    void* argtable[] = {
        help, version, random, dly, manual, name, idx, file, end
    };

    dly->ival[0] = 500;

    int argerror_count = arg_parse(argc, argv, argtable);

    if (0 != help->count) {
        usage(argv[0], argtable);
        return EXIT_SUCCESS;
    }

    if (0 != version->count) {
        fprintf(stderr, "%d.%d.%d\n", CE_SPIKE_VERSION_MAJOR,
                                        CE_SPIKE_VERSION_MINOR,
                                        CE_SPIKE_VERSION_PATCH);
        return EXIT_SUCCESS;
    }

    if (0 != argerror_count) {
        usage(argv[0], argtable);
        arg_print_errors(stderr, end, argv[0]);
        return EXIT_FAILURE;
    }

    ce_logging_init();
    ce_alloc_init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE);

    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 300);
    glutCreateWindow("Cursed Earth: Texture Viewer");

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    ce_input_init();
    ce_gl_init();

    res = ce_resfile_open_file(file->sval[0]);
    if (NULL == res) {
        ce_logging_fatal("main: could not open file: '%s'", file->sval[0]);
        return EXIT_FAILURE;
    }

    rndmode = 0 != random->count;
    delay = ce_max(0, dly->ival[0]);

    if (0 != name->count || 0 != idx->count || 0 != manual->count) {
        slideshow = false;
    }

    index = -1;
    srand(time(NULL));

    if (0 != name->count) {
        index = ce_resfile_node_index(res, name->sval[0]);
        if (-1 == index) {
            ce_logging_error("main: could not find texture: '%s'\n", name->sval[0]);
            return EXIT_FAILURE;
        }
    } else if (0 != idx->count) {
        index = idx->ival[0];
        if (index < 0) {
            index = INT_MAX;
        }
        if (0 > index || index >= (int)res->node_count) {
            ce_logging_error("main: invalid index: %d, "
                            "allowed range [0...%d]\n",
                            index, res->node_count - 1);
            return EXIT_FAILURE;
        }
    } else {
        index = rndmode ? rand() % res->node_count : 0;
    }

    glutTimerFunc(0, next_texture, 0);

    tmr = ce_timer_new();

    es = ce_input_event_supply_new();
    next_texture_event = ce_input_event_supply_single_front_event(es,
                    ce_input_event_supply_button_event(es, CE_KB_N));

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    glutMainLoop();*/
    return EXIT_SUCCESS;
}
