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

ce_figfile_proto* fig_proto;
ce_resfile* res;
ce_texture* tex;
ce_camera* cam;
ce_timer* tmr;

ce_input_event_supply* es;
ce_input_event* ev;

int counter;

static void debug_print()
{
	printf("unknown1: %d\n", fig_proto->unknown1);
	printf("unknown2: %d\n", fig_proto->unknown2);
	printf("\ncenter:\n");
	for (int i = 0; i < 8; ++i) {
		printf("\t%f %f %f\n", fig_proto->center[i].x, fig_proto->center[i].y, fig_proto->center[i].z);
	}
	printf("\nmin:\n");
	for (int i = 0; i < 8; ++i) {
		printf("\t%f %f %f\n", fig_proto->min[i].x, fig_proto->min[i].y, fig_proto->min[i].z);
	}
	printf("\nmax:\n");
	for (int i = 0; i < 8; ++i) {
		printf("\t%f %f %f\n", fig_proto->max[i].x, fig_proto->max[i].y, fig_proto->max[i].z);
	}
	printf("\nradius:\n");
	for (int i = 0; i < 8; ++i) {
		printf("\t%f\n", fig_proto->radius[i]);
	}
	printf("\nvertices (%d):\n\n", fig_proto->vertex_count);
	for (int i = 0; i < fig_proto->vertex_count; ++i) {
		printf("\tvertex %d:\n", i + 1);
		for (int j = 0; j < 8; ++j) {
			printf("\t\tx: %f %f %f %f\n", fig_proto->vertices[i].x[j][0], fig_proto->vertices[i].x[j][1], fig_proto->vertices[i].x[j][2], fig_proto->vertices[i].x[j][3]);
			printf("\t\ty: %f %f %f %f\n", fig_proto->vertices[i].y[j][0], fig_proto->vertices[i].y[j][1], fig_proto->vertices[i].y[j][2], fig_proto->vertices[i].y[j][3]);
			printf("\t\tz: %f %f %f %f\n\n", fig_proto->vertices[i].z[j][0], fig_proto->vertices[i].z[j][1], fig_proto->vertices[i].z[j][2], fig_proto->vertices[i].z[j][3]);
		}
	}
	printf("\nnormals (%d):\n", fig_proto->normal_count);
	for (int i = 0; i < fig_proto->normal_count; ++i) {
		printf("\tx: %f %f %f %f\n", fig_proto->normals[i].x[0], fig_proto->normals[i].x[1], fig_proto->normals[i].x[2], fig_proto->normals[i].x[3]);
		printf("\ty: %f %f %f %f\n", fig_proto->normals[i].y[0], fig_proto->normals[i].y[1], fig_proto->normals[i].y[2], fig_proto->normals[i].y[3]);
		printf("\tz: %f %f %f %f\n", fig_proto->normals[i].z[0], fig_proto->normals[i].z[1], fig_proto->normals[i].z[2], fig_proto->normals[i].z[3]);
		printf("\tw: %f %f %f %f\n\n", fig_proto->normals[i].w[0], fig_proto->normals[i].w[1], fig_proto->normals[i].w[2], fig_proto->normals[i].w[3]);
	}
	printf("texcoords (%d):\n", fig_proto->texcoord_count);
	for (int i = 0; i < fig_proto->texcoord_count; ++i) {
		printf("\t%f %f\n", fig_proto->texcoords[i].x, fig_proto->texcoords[i].y);
	}
	printf("\nindices (%d):\n\t", fig_proto->index_count);
	for (int i = 0; i < fig_proto->index_count; ++i) {
		printf("%hd ", fig_proto->indices[i]);
	}
	printf("\n\ncomponents (%d):\n\t", fig_proto->component_count);
	for (int i = 0; i < fig_proto->component_count; ++i) {
		printf("(%hd, %hd, %hd) ", fig_proto->components[i].vertex_index,
			fig_proto->components[i].normal_index, fig_proto->components[i].texcoord_index);
	}
	printf("\n\nlight components (%d):\n\t", fig_proto->light_component_count);
	for (int i = 0; i < fig_proto->light_component_count; ++i) {
		printf("(%hd, %hd) ", fig_proto->light_components[i].unknown1,
								fig_proto->light_components[i].unknown2);
	}
	printf("\n");
}

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
		ce_figfile_proto_close(fig_proto);
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

	if (ce_input_event_triggered(ev)) {
		++counter;
	}

	glutPostRedisplay();
}

static float fig8_value(const float* params, const ce_complection* cm)
{
	float value, temp[2];
	temp[0] = params[0] + (params[1] - params[0]) * cm->strength;
	temp[1] = params[2] + (params[3] - params[2]) * cm->strength;
	value = temp[0] + (temp[1] - temp[0]) * cm->dexterity;
	temp[0] = params[4] + (params[5] - params[4]) * cm->strength;
	temp[1] = params[6] + (params[7] - params[6]) * cm->strength;
	temp[0] += (temp[1] - temp[0]) * cm->dexterity;
	value += (temp[0] - value) * cm->height;
	return value;
}

static void display(void)
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();
	ce_camera_setup(cam);

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

	float xbuff[8], ybuff[8], zbuff[8];

	float cmval = counter % 10 / 10.0f;
	ce_complection cm = { cmval, cmval, cmval };

	glPushMatrix();
	for (int i = 0; i < 8; i++) {
		xbuff[i] = fig_proto->center[i].x;
		ybuff[i] = fig_proto->center[i].y;
		zbuff[i] = fig_proto->center[i].z;
	}
	glTranslatef(fig8_value(xbuff, &cm),
				fig8_value(ybuff, &cm),
				-1.0f * fig8_value(zbuff, &cm));
	glutWireCube(2.0f * fig8_value(fig_proto->radius, &cm));
	glPopMatrix();

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	ce_texture_bind(tex);

	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < fig_proto->index_count; ++i) {
		ce_figfile_proto_component cp = fig_proto->components[fig_proto->indices[i]];

		glTexCoord2f(fig_proto->texcoords[cp.texcoord_index].x,
			fig_proto->texcoords[cp.texcoord_index].y);

		ce_figfile_proto_normal* nor = fig_proto->normals + cp.normal_index / 4;
		ce_figfile_proto_vertex* ver = fig_proto->vertices + cp.vertex_index / 4;

		float nw = nor->w[cp.normal_index % 4];

		glNormal3f(nor->x[cp.normal_index % 4] / nw,
			nor->y[cp.normal_index % 4] / nw,
			nor->z[cp.normal_index % 4] / nw);

		for (int j = 0; j < 8; j++) {
			xbuff[j] = ver->x[j][cp.vertex_index % 4];
			ybuff[j] = ver->y[j][cp.vertex_index % 4];
			zbuff[j] = ver->z[j][cp.vertex_index % 4];
		}

		glVertex3f(fig8_value(xbuff, &cm),
					fig8_value(ybuff, &cm),
					-1.0f * fig8_value(zbuff, &cm));
	}
	glEnd();

	ce_texture_unbind(tex);

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

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

	fig_proto = ce_figfile_proto_open_file(argv[2]);
	if (NULL == fig_proto) {
		printf("main: failed to load fig: '%s'\n", argv[2]);
		return EXIT_FAILURE;
	}

	debug_print(fig_proto);

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
	ev = ce_input_event_supply_single_front_event(es,
		ce_input_event_supply_button_event(es, CE_KB_N));

	glutMainLoop();
	return EXIT_SUCCESS;
}
