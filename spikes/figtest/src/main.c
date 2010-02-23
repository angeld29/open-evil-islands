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
#include <stdint.h>
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
#include "cevector.h"
#include "cestring.h"
#include "ceresfile.h"
#include "cemprfile.h"
#include "cecfgfile.h"
#include "celightcfg.h"
#include "cefigfile.h"
#include "cetexture.h"
#include "cecomplection.h"

ce_resfile* fig_res;
ce_resfile* tex_res;
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

typedef struct fig_node fig_node;

struct fig_node {
	ce_string* name;
	ce_figfile* fig;
	ce_vec3 bone;
	ce_vector* childs;
};

fig_node* root_node;

void clear_fig(fig_node* node)
{
	if (NULL != node) {
		for (int i = 0, n = ce_vector_count(node->childs); i < n; ++i) {
			fig_node* child_node = ce_vector_at(node->childs, i);
			clear_fig(child_node);
		}
		ce_vector_del(node->childs);
		ce_figfile_close(node->fig);
		ce_string_del(node->name);
		ce_free(node, sizeof(fig_node));
	}
}

static fig_node* find_node(ce_vector* nodes, const char* name)
{
	for (int i = 0, n = ce_vector_count(nodes); i < n; ++i) {
		fig_node* node = ce_vector_at(nodes, i);
		if (0 == strcmp(name, ce_string_cstr(node->name))) {
			return node;
		}
	}
	return NULL;
}

static bool generate_texture(const char* name)
{
	int index = ce_resfile_node_index(tex_res, name);
	if (-1 == index) {
		return false;
	}

	void* data = ce_alloc(ce_resfile_node_size(tex_res, index));
	if (!ce_resfile_node_data(tex_res, index, data)) {
		ce_free(data, ce_resfile_node_size(tex_res, index));
		return false;
	}

	ce_texture_del(tex);
	tex = ce_texture_new(data);
	ce_free(data, ce_resfile_node_size(tex_res, index));

	return NULL != tex;
}

static float value_fig8(const float* params, int stride,
							const ce_complection* cm)
{
	float temp1 = params[0 * stride] + (params[1 * stride] -
										params[0 * stride]) * cm->strength;
	float temp2 = params[2 * stride] + (params[3 * stride] -
										params[2 * stride]) * cm->strength;
	float value = temp1 + (temp2 - temp1) * cm->dexterity;
	temp1 = params[4 * stride] + (params[5 * stride] -
								params[4 * stride]) * cm->strength;
	temp2 = params[6 * stride] + (params[7 * stride] -
								params[6 * stride]) * cm->strength;
	temp1 += (temp2 - temp1) * cm->dexterity;
	return value += (temp1 - value) * cm->height;
}

static bool make_fig(const char* name)
{
	ce_complection cm = {
		.strength = counter_strength % 11 / 10.0f,
		.dexterity = counter_dexterity % 11 / 10.0f,
		.height = counter_height % 11 / 10.0f
	};

	char mod_name[16], bon_name[16];
	snprintf(mod_name, sizeof(mod_name), "%s.mod", name);
	snprintf(bon_name, sizeof(bon_name), "%s.bon", name);

	int mod_index = ce_resfile_node_index(fig_res, mod_name);
	int bon_index = ce_resfile_node_index(fig_res, bon_name);

	if (-1 == mod_index || -1 == bon_index) {
		return false;
	}

	void* mod_data = ce_alloc(ce_resfile_node_size(fig_res, mod_index));
	void* bon_data = ce_alloc(ce_resfile_node_size(fig_res, bon_index));

	ce_resfile_node_data(fig_res, mod_index, mod_data);
	ce_resfile_node_data(fig_res, bon_index, bon_data);

	ce_memfile* mod_mem = ce_memfile_open_data(mod_data, ce_resfile_node_size(fig_res, mod_index), "rb");
	ce_memfile* bon_mem = ce_memfile_open_data(bon_data, ce_resfile_node_size(fig_res, bon_index), "rb");

	ce_resfile* mod_res = ce_resfile_open_memfile(mod_name, mod_mem);
	ce_resfile* bon_res = ce_resfile_open_memfile(bon_name, bon_mem);

	ce_vector* nodes = ce_vector_new();

	for (int i = 0, n = ce_resfile_node_count(mod_res); i < n; ++i) {
		const char* part_name = ce_resfile_node_name(mod_res, i);

		if (0 == strcmp(name, part_name)) {
			continue;
		}

		void* fig_data = ce_alloc(ce_resfile_node_size(mod_res, i));
		ce_resfile_node_data(mod_res, i, fig_data);
		ce_memfile* fig_mem = ce_memfile_open_data(fig_data, ce_resfile_node_size(mod_res, i), "rb");

		ce_figfile_proto* proto = ce_figfile_proto_open_memfile(fig_mem);
		ce_figfile* fig = ce_figfile_open(proto, &cm);

		int bon_index = ce_resfile_node_index(bon_res, part_name);
		float* bon_data = ce_alloc(ce_resfile_node_size(bon_res, bon_index));
		ce_resfile_node_data(bon_res, bon_index, bon_data);

		fig_node* node = ce_alloc(sizeof(fig_node));
		node->name = ce_string_new_cstr(part_name);
		node->fig = fig;
		node->childs = ce_vector_new();
		ce_vec3_init(&node->bone,
					value_fig8(bon_data + 0, 3, &cm),
					value_fig8(bon_data + 1, 3, &cm),
					value_fig8(bon_data + 2, 3, &cm));

		ce_vector_push_back(nodes, node);

		ce_free(bon_data, ce_resfile_node_size(bon_res, bon_index));
		ce_figfile_proto_close(proto);
		ce_memfile_close(fig_mem);
		ce_free(fig_data, ce_resfile_node_size(mod_res, i));
	}

	int lnk_index = ce_resfile_node_index(mod_res, name);
	if (-1 == lnk_index) {
		return false;
	}
	void* lnk_data = ce_alloc(ce_resfile_node_size(mod_res, lnk_index));
	char* lnk = lnk_data;
	ce_resfile_node_data(mod_res, lnk_index, lnk_data);

	int pair_count = *(uint32_t*)lnk;
	lnk += sizeof(uint32_t);
	for (int i = 0; i < pair_count; ++i) {
		int length = *(uint32_t*)lnk;
		lnk += sizeof(uint32_t);
		ce_string* name_child = ce_string_new_cstr_n(lnk, length);
		lnk += length;
		length = *(uint32_t*)lnk;
		lnk += sizeof(uint32_t);
		ce_string* name_parent = ce_string_new_cstr_n(lnk, length);
		lnk += length;

		fig_node* node_parent = find_node(nodes, ce_string_cstr(name_parent));
		fig_node* node_child = find_node(nodes, ce_string_cstr(name_child));

		if (NULL == node_parent) {
			assert(NULL == root_node);
			assert(NULL != node_child);
			root_node = node_child;
		} else {
			assert(NULL != node_parent);
			assert(NULL != node_child);
			ce_vector_push_back(node_parent->childs, node_child);
		}

		ce_string_del(name_parent);
		ce_string_del(name_child);
	}

	ce_free(lnk_data, ce_resfile_node_size(mod_res, lnk_index));
	ce_vector_del(nodes);
	ce_resfile_close(bon_res);
	ce_resfile_close(mod_res);
	ce_free(mod_data, ce_resfile_node_size(fig_res, mod_index));
	ce_free(bon_data, ce_resfile_node_size(fig_res, bon_index));

	return true;
}

static void idle(void)
{
	ce_timer_advance(tmr);

	float elapsed = ce_timer_elapsed(tmr);

	ce_input_advance(elapsed);
	ce_input_event_supply_advance(es, elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		clear_fig(root_node);
		ce_input_event_supply_del(es);
		ce_timer_del(tmr);
		ce_camera_del(cam);
		ce_texture_del(tex);
		ce_resfile_close(tex_res);
		ce_resfile_close(fig_res);
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

static void render_fig(fig_node* node)
{
	glPushMatrix();
	glTranslatef(node->bone.x, node->bone.y, -1.0f * node->bone.z);

	glPushMatrix();
	glTranslatef(node->fig->bounding_box.center.x,
				node->fig->bounding_box.center.y,
				-1.0f * node->fig->bounding_box.center.z);
	glutWireCube(2.0f * node->fig->radius);
	glPopMatrix();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	ce_texture_bind(tex);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < node->fig->index_count; ++i) {
		ce_figfile_component cp = node->fig->components[node->fig->indices[i]];

		glTexCoord2f(node->fig->texcoords[cp.texcoord_index].x,
					node->fig->texcoords[cp.texcoord_index].y);

		glNormal3f(node->fig->normals[cp.normal_index].x,
					node->fig->normals[cp.normal_index].y,
					-1.0f * node->fig->normals[cp.normal_index].z);

		glVertex3f(node->fig->vertices[cp.vertex_index].x,
					node->fig->vertices[cp.vertex_index].y,
					-1.0f * node->fig->vertices[cp.vertex_index].z);
	}
	glEnd();

	ce_texture_unbind(tex);

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);

	for (int i = 0, n = ce_vector_count(node->childs); i < n; ++i) {
		fig_node* child_node = ce_vector_at(node->childs, i);
		render_fig(child_node);
	}

	glPopMatrix();
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

	glPushMatrix();
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	if (NULL != root_node) {
		render_fig(root_node);
	}
	glPopMatrix();

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

	tex_res = ce_resfile_open_file(argv[1]);
	if (NULL == tex_res) {
		printf("main: could not open file: '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (!generate_texture(argv[2])) {
		printf("main: could not generate texture\n");
		return EXIT_FAILURE;
	}

	fig_res = ce_resfile_open_file(argv[3]);
	if (NULL == fig_res) {
		printf("main: could not open file: '%s'\n", argv[3]);
		return EXIT_FAILURE;
	}

	if (!make_fig(argv[4])) {
		printf("main: could not load model: '%s'\n", argv[4]);
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
