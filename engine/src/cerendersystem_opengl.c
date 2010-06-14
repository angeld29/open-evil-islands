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
#include <assert.h>

#include "celib.h"
#include "cemath.h"
#include "cemat4.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceopengl.h"
#include "cerendersystem.h"

typedef struct {
	ce_mat4 view;
	GLuint axes_list;
	GLuint wire_cube_list;
	GLuint solid_cube_list;
} ce_opengl_system;

ce_rendersystem* ce_rendersystem_new(void)
{
	ce_logging_write("render system: %s", glGetString(GL_VENDOR));
	ce_logging_write("render system: %s", glGetString(GL_RENDERER));
	ce_logging_write("render system: using GL %s", glGetString(GL_VERSION));
	ce_logging_write("render system: using GLU %s", gluGetString(GLU_VERSION));
	ce_logging_write("render system: using GLEW %s", glewGetString(GLEW_VERSION));

	struct {
		const char* name;
		bool available;
	} extensions[] = {
		{ "ARB texture non power of two", GLEW_ARB_texture_non_power_of_two },
		{ "EXT texture compression s3tc", GLEW_EXT_texture_compression_s3tc },
		{ "EXT texture compression dxt1", GLEW_EXT_texture_compression_dxt1 },
		{ "EXT texture edge clamp", GLEW_EXT_texture_edge_clamp },
		{ "SGIS texture edge clamp", GLEW_SGIS_texture_edge_clamp },
		{ "SGIS texture lod", GLEW_SGIS_texture_lod },
		{ "SGIS generate mipmap", GLEW_SGIS_generate_mipmap },
		{ "EXT packed pixels", GLEW_EXT_packed_pixels },
		{ "ARB occlusion query 2", GLEW_ARB_occlusion_query2 },
		{ "AMD vertex shader tessellator", GLEW_AMD_vertex_shader_tessellator },
		{ "AMD performance monitor", GLEW_AMD_performance_monitor },
		{ "AMD meminfo", GLEW_ATI_meminfo },
	};

	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
		ce_logging_write("render system: checking for '%s' extension... %s",
			extensions[i].name, extensions[i].available ? "yes" : "no");
	}

	ce_rendersystem* rendersystem = ce_alloc(sizeof(ce_rendersystem) +
												sizeof(ce_opengl_system));
	ce_opengl_system* opengl_system = (ce_opengl_system*)rendersystem->impl;

	rendersystem->thread_id = ce_thread_self();
	opengl_system->view = CE_MAT4_IDENTITY;
	opengl_system->axes_list = glGenLists(1);
	opengl_system->wire_cube_list = glGenLists(1);
	opengl_system->solid_cube_list = glGenLists(1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glNewList(opengl_system->axes_list, GL_COMPILE);
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();
	glEndList();

	glNewList(opengl_system->wire_cube_list, GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	// face 1 front xy plane
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f,  1.0f,  1.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	// face 2 right yz plane
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f, -1.0f, -1.0f);
	glVertex3f( 1.0f,  1.0f, -1.0f);
	glVertex3f( 1.0f,  1.0f,  1.0f);
	// face 3 top xz plane
	glVertex3f( 1.0f,  1.0f, -1.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	// face 4 left yz plane
	glVertex3f(-1.0f, -1.0f,  1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	// face 5 back xy plane
	glVertex3f( 1.0f,  1.0f, -1.0f);
	glVertex3f( 1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	// face 6 bottom xz plane
	glVertex3f(-1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f, -1.0f, -1.0f);
	glEnd();
	glEndList();

	glNewList(opengl_system->solid_cube_list, GL_COMPILE);
	glBegin(GL_QUADS);
	// face 1 front xy plane
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f,  1.0f,  1.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	// face 2 right yz plane
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f, -1.0f, -1.0f);
	glVertex3f( 1.0f,  1.0f, -1.0f);
	glVertex3f( 1.0f,  1.0f,  1.0f);
	// face 3 top xz plane
	glVertex3f( 1.0f,  1.0f,  1.0f);
	glVertex3f( 1.0f,  1.0f, -1.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	// face 4 left yz plane
	glVertex3f(-1.0f,  1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	// face 5 back xy plane
	glVertex3f(-1.0f,  1.0f, -1.0f);
	glVertex3f( 1.0f,  1.0f, -1.0f);
	glVertex3f( 1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	// face 6 bottom xz plane
	glVertex3f( 1.0f, -1.0f, -1.0f);
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glEnd();
	glEndList();

	return rendersystem;
}

void ce_rendersystem_del(ce_rendersystem* rendersystem)
{
	if (NULL != rendersystem) {
		ce_opengl_system* opengl_system = (ce_opengl_system*)rendersystem->impl;
		glDeleteLists(opengl_system->solid_cube_list, 1);
		glDeleteLists(opengl_system->wire_cube_list, 1);
		glDeleteLists(opengl_system->axes_list, 1);
		ce_free(rendersystem, sizeof(ce_rendersystem) + sizeof(ce_opengl_system));
	}
}

void ce_rendersystem_begin_render(ce_rendersystem* rendersystem,
									const ce_color* clear_color)
{
	ce_unused(rendersystem);

	glClearColor(clear_color->r, clear_color->g,
				clear_color->b, clear_color->a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
}

void ce_rendersystem_end_render(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

#if 0
	if (ce_gl_query_feature(CE_GL_FEATURE_MEMINFO)) {
		const int n = 3;
		GLint params[4], viewport[4];
		GLenum pnames[] = { CE_GL_VBO_FREE_MEMORY,
							CE_GL_TEXTURE_FREE_MEMORY,
							CE_GL_RENDERBUFFER_FREE_MEMORY };
		const char* names[] = { "VBO", "TEX", "RB" };

		glGetIntegerv(GL_VIEWPORT, viewport);

		for (int i = 0; i < n; ++i) {
			glGetIntegerv(pnames[i], params);
			printf("%s: %.3f MB free\n", names[i], params[0] / 1000.0f);
		}
	}
#endif

#if 0
	static bool flag = true;
	if (flag && ce_gl_query_feature(CE_GL_FEATURE_PERFORMANCE_MONITOR)) {
		flag = false;
		GLint group_count;
		ce_gl_get_perfmon_groups(&group_count, 0, NULL);

		GLuint groups[group_count];
		ce_gl_get_perfmon_groups(NULL, group_count, groups);

		for (int i = 0; i < group_count; ++i) {
			GLsizei length;
			ce_gl_get_perfmon_group_str(groups[i], 0, &length, NULL);

			char group_name[length + 1];
			ce_gl_get_perfmon_group_str(groups[i],
				sizeof(group_name), NULL, group_name);

			GLint counter_count, max_active;
			ce_gl_get_perfmon_counters(groups[i], &counter_count,
										&max_active, 0, NULL);

			GLuint counters[counter_count];
			ce_gl_get_perfmon_counters(groups[i], NULL, NULL,
										counter_count, counters);

			printf("group: %s, max active: %d\n", group_name, max_active);

			for (int j = 0; j < counter_count; ++j) {
				GLsizei length;
				ce_gl_get_perfmon_counter_str(groups[i],
					counters[j], 0, &length, NULL);

				char counter_name[length + 1];
				ce_gl_get_perfmon_counter_str(groups[i], counters[j],
					sizeof(counter_name), NULL, counter_name);

				printf("\tcounter: %s\n", counter_name);
			}
			printf("\n");
		}
	}
#endif

	ce_opengl_report_errors();

	glFlush();
}

void ce_rendersystem_draw_axes(ce_rendersystem* rendersystem)
{
	ce_opengl_system* opengl_system = (ce_opengl_system*)rendersystem->impl;
	glCallList(opengl_system->axes_list);
}

void ce_rendersystem_draw_wire_cube(ce_rendersystem* rendersystem)
{
	ce_opengl_system* opengl_system = (ce_opengl_system*)rendersystem->impl;
	glCallList(opengl_system->wire_cube_list);
}

void ce_rendersystem_draw_solid_cube(ce_rendersystem* rendersystem)
{
	ce_opengl_system* opengl_system = (ce_opengl_system*)rendersystem->impl;
	glCallList(opengl_system->solid_cube_list);
}

void ce_rendersystem_draw_video_frame(ce_rendersystem* rendersystem, ce_texture* texture)
{
	ce_unused(rendersystem);

	ce_texture_bind(texture);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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

void ce_rendersystem_setup_viewport(ce_rendersystem* rendersystem,
										ce_viewport* viewport)
{
	ce_unused(rendersystem);

	glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
	glScissor(viewport->x, viewport->y, viewport->width, viewport->height);
}

void ce_rendersystem_setup_camera(ce_rendersystem* rendersystem,
										ce_camera* camera)
{
	ce_opengl_system* opengl_system = (ce_opengl_system*)rendersystem->impl;

	float tx  = 2.0f * camera->orientation.x;
	float ty  = 2.0f * camera->orientation.y;
	float tz  = 2.0f * camera->orientation.z;
	float twx = tx * camera->orientation.w;
	float twy = ty * camera->orientation.w;
	float twz = tz * camera->orientation.w;
	float txx = tx * camera->orientation.x;
	float txy = ty * camera->orientation.x;
	float txz = tz * camera->orientation.x;
	float tyy = ty * camera->orientation.y;
	float tyz = tz * camera->orientation.y;
	float tzz = tz * camera->orientation.z;

	opengl_system->view.m[0] = 1.0f - (tyy + tzz);
	opengl_system->view.m[1] = txy + twz;
	opengl_system->view.m[2] = txz - twy;

	opengl_system->view.m[4] = txy - twz;
	opengl_system->view.m[5] = 1.0f - (txx + tzz);
	opengl_system->view.m[6] = tyz + twx;

	opengl_system->view.m[8] = txz + twy;
	opengl_system->view.m[9] = tyz - twx;
	opengl_system->view.m[10] = 1.0f - (txx + tyy);

	opengl_system->view.m[12] = -
		opengl_system->view.m[0] * camera->position.x -
		opengl_system->view.m[4] * camera->position.y -
		opengl_system->view.m[8] * camera->position.z;

	opengl_system->view.m[13] = -
		opengl_system->view.m[1] * camera->position.x -
		opengl_system->view.m[5] * camera->position.y -
		opengl_system->view.m[9] * camera->position.z;

	opengl_system->view.m[14] = -
		opengl_system->view.m[2] * camera->position.x -
		opengl_system->view.m[6] * camera->position.y -
		opengl_system->view.m[10] * camera->position.z;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera->fov, camera->aspect, camera->near, camera->far);

	glMatrixMode(GL_MODELVIEW);
	glMultMatrixf(opengl_system->view.m);
}

void ce_rendersystem_begin_occlusion_test(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glDisable(GL_CULL_FACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
}

void ce_rendersystem_end_occlusion_test(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_CULL_FACE);
}

void ce_rendersystem_apply_color(ce_rendersystem* rendersystem,
									const ce_color* color)
{
	ce_unused(rendersystem);

	glColor4f(color->r, color->g, color->b, color->a);
}

void ce_rendersystem_apply_transform(ce_rendersystem* rendersystem,
									const ce_vec3* translation,
									const ce_quat* rotation,
									const ce_vec3* scaling)
{
	ce_unused(rendersystem);

	ce_vec3 axis;
	float angle = ce_quat_to_polar(rotation, &axis);

	glPushMatrix();

	glTranslatef(translation->x, translation->y, translation->z);
	glRotatef(ce_rad2deg(angle), axis.x, axis.y, axis.z);
	glScalef(scaling->x, scaling->y, scaling->z);
}

void ce_rendersystem_discard_transform(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glPopMatrix();
}

void ce_rendersystem_apply_material(ce_rendersystem* rendersystem,
										ce_material* material)
{
	ce_unused(rendersystem);

	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_LIGHTING);

	glMaterialfv(GL_FRONT, GL_AMBIENT, (float[]) { material->ambient.r,
													material->ambient.g,
													material->ambient.b,
													material->ambient.a });
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[]) { material->diffuse.r,
													material->diffuse.g,
													material->diffuse.b,
													material->diffuse.a });
	glMaterialfv(GL_FRONT, GL_SPECULAR, (float[]) { material->specular.r,
													material->specular.g,
													material->specular.b,
													material->specular.a });
	glMaterialfv(GL_FRONT, GL_EMISSION, (float[]) { material->emission.r,
													material->emission.g,
													material->emission.b,
													material->emission.a });
	glMaterialf(GL_FRONT, GL_SHININESS, material->shininess);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
		(GLint[]){ GL_MODULATE, GL_DECAL, GL_REPLACE }[material->mode]);

	if (material->alpha_test) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5f);
	}

	if (material->blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void ce_rendersystem_discard_material(ce_rendersystem* rendersystem,
										ce_material* material)
{
	ce_unused(rendersystem), ce_unused(material);

	glPopAttrib();
}
