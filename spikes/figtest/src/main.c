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

#include "cegl.h"
#include "celogging.h"
#include "cealloc.h"
#include "ceinput.h"
#include "cetimer.h"
#include "cecamera.h"
#include "cemath.h"
#include "cevec3.h"
#include "cefrustum.h"
#include "ceresfile.h"
#include "cemprfile.h"
#include "cecfgfile.h"
#include "celightcfg.h"
#include "cefigfile.h"
#include "cetexture.h"
#include "cecomplection.h"

ce_figfile_proto* proto;
ce_resfile* res;
ce_texture* tex;
ce_camera* cam;
ce_timer* tmr;

ce_input_event_supply* es;
ce_input_event* ev_strength;
ce_input_event* ev_dexterity;
ce_input_event* ev_height;

int counter_strength;
int counter_dexterity;
int counter_height;

static bool generate_texture(const char* name)
{
	int index = ce_resfile_node_index(res, name);
	if (-1 == index) {
		return false;
	}

	void* data = ce_alloc(ce_resfile_node_size(res, index));
	if (!ce_resfile_node_data(res, index, data)) {
		ce_free(data, ce_resfile_node_size(res, index));
		return false;
	}

	ce_texture_del(tex);
	tex = ce_texture_new(data);
	ce_free(data, ce_resfile_node_size(res, index));

	return NULL != tex;
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
		ce_camera_del(cam);
		ce_texture_del(tex);
		ce_resfile_close(res);
		ce_figfile_proto_close(proto);
		ce_gl_term();
		ce_input_term();
		ce_alloc_term();
		ce_logging_term();
		exit(EXIT_SUCCESS);
	}

	if (ce_input_test(CE_KB_LEFT)) {
		ce_camera_move(cam, -0.5f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_UP)) {
		ce_camera_move(cam, 0.0f, 0.5f * elapsed);
	}

	if (ce_input_test(CE_KB_RIGHT)) {
		ce_camera_move(cam, 0.5f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_DOWN)) {
		ce_camera_move(cam, 0.0f, -0.5f * elapsed);
	}

	if (ce_input_test(CE_MB_WHEELUP)) {
		ce_camera_zoom(cam, 0.1f);
	}

	if (ce_input_test(CE_MB_WHEELDOWN)) {
		ce_camera_zoom(cam, -0.1f);
	}

	if (ce_input_test(CE_MB_RIGHT)) {
		ce_vec2 offset = ce_input_mouse_offset();
		ce_camera_yaw_pitch(cam, ce_deg2rad(-0.13f * offset.x),
									ce_deg2rad(-0.13f * offset.y));
	}

	if (ce_input_event_triggered(ev_strength)) {
		++counter_strength;
	}

	if (ce_input_event_triggered(ev_dexterity)) {
		++counter_dexterity;
	}

	if (ce_input_event_triggered(ev_height)) {
		++counter_height;
	}

	glutPostRedisplay();
}

static void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	ce_camera_setup(cam);

	glEnable(GL_DEPTH_TEST);

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);
	glEnd();
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	glEnd();
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();

	ce_complection cm = {
		.strength = counter_strength % 11 / 10.0f,
		.dexterity = counter_dexterity % 11 / 10.0f,
		.height = counter_height % 11 / 10.0f
	};

	ce_figfile* fig = ce_figfile_open(proto, &cm);

	glPushMatrix();
	glTranslatef(fig->bounding_box.center.x,
				fig->bounding_box.center.y,
				-1.0f * fig->bounding_box.center.z);
	glutWireCube(2.0f * fig->radius);
	glPopMatrix();

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	ce_texture_bind(tex);

	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < fig->index_count; ++i) {
		ce_figfile_component cp = fig->components[fig->indices[i]];

		glTexCoord2f(fig->texcoords[cp.texcoord_index].x,
					fig->texcoords[cp.texcoord_index].y);

		glNormal3f(fig->normals[cp.normal_index].x,
					fig->normals[cp.normal_index].y,
					-1.0f * fig->normals[cp.normal_index].z);

		glVertex3f(fig->vertices[cp.vertex_index].x,
					fig->vertices[cp.vertex_index].y,
					-1.0f * fig->vertices[cp.vertex_index].z);
	}
	glEnd();

	ce_texture_unbind(tex);

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	ce_figfile_close(fig);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	ce_camera_set_aspect(cam, width, height);
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

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Cursed Earth: Figure Viewer");

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_init();
	ce_gl_init();

	proto = ce_figfile_proto_open_file(argv[2]);
	if (NULL == proto) {
		printf("main: failed to load fig: '%s'\n", argv[2]);
		return EXIT_FAILURE;
	}

	res = ce_resfile_open_file(argv[1]);
	if (NULL == res) {
		printf("main: could not open file: '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (!generate_texture(argv[3])) {
		printf("main: could not generate texture\n");
		return EXIT_FAILURE;
	}

	ce_vec3 eye;
	ce_vec3_init(&eye, -1.0f, 1.0f, 1.0f);

	cam = ce_camera_new();
	ce_camera_set_near(cam, 0.1f);
	ce_camera_set_eye(cam, &eye);
	ce_camera_yaw_pitch(cam, ce_deg2rad(45.0f), ce_deg2rad(30.0f));

	tmr = ce_timer_new();
	es = ce_input_event_supply_new();
	ev_strength = ce_input_event_supply_single_front_event(es,
		ce_input_event_supply_button_event(es, CE_KB_1));
	ev_dexterity = ce_input_event_supply_single_front_event(es,
		ce_input_event_supply_button_event(es, CE_KB_2));
	ev_height = ce_input_event_supply_single_front_event(es,
		ce_input_event_supply_button_event(es, CE_KB_3));

	glutMainLoop();
	return EXIT_SUCCESS;
}
