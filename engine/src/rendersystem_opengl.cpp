/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>

#include "lib.hpp"
#include "math.hpp"
#include "mat4.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "opengl.hpp"
#include "rendersystem.hpp"

namespace cursedearth
{
    struct ce_render_system {
        ce_thread_id thread_id;
        ce_mat4 view;
        GLuint axes_list;
        GLuint wire_cube_list;
        GLuint solid_cube_list;
        GLuint solid_sphere_list;
    }* ce_render_system;

    void ce_render_system_init(void)
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
            { "ARB texture non power of two", static_cast<bool>(GLEW_ARB_texture_non_power_of_two) },
            { "EXT texture compression s3tc", static_cast<bool>(GLEW_EXT_texture_compression_s3tc) },
            { "EXT texture compression dxt1", static_cast<bool>(GLEW_EXT_texture_compression_dxt1) },
            { "EXT texture edge clamp", static_cast<bool>(GLEW_EXT_texture_edge_clamp) },
            { "SGIS texture edge clamp", static_cast<bool>(GLEW_SGIS_texture_edge_clamp) },
            { "SGIS texture lod", static_cast<bool>(GLEW_SGIS_texture_lod) },
            { "SGIS generate mipmap", static_cast<bool>(GLEW_SGIS_generate_mipmap) },
            { "EXT packed pixels", static_cast<bool>(GLEW_EXT_packed_pixels) },
            { "ARB occlusion query 2", static_cast<bool>(GLEW_ARB_occlusion_query2) },
            { "AMD vertex shader tessellator", static_cast<bool>(GLEW_AMD_vertex_shader_tessellator) },
            { "AMD performance monitor", static_cast<bool>(GLEW_AMD_performance_monitor) },
            { "AMD meminfo", static_cast<bool>(GLEW_ATI_meminfo) },
        };

        for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
            ce_logging_write("render system: checking for '%s' extension... %s",
                extensions[i].name, extensions[i].available ? "yes" : "no");
        }

        ce_render_system = (struct ce_render_system*)ce_alloc_zero(sizeof(struct ce_render_system));
        ce_render_system->thread_id = ce_thread_self();
        ce_render_system->view = CE_MAT4_IDENTITY;
        ce_render_system->axes_list = glGenLists(1);
        ce_render_system->wire_cube_list = glGenLists(1);
        ce_render_system->solid_cube_list = glGenLists(1);
        ce_render_system->solid_sphere_list = glGenLists(1);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        GLUquadric* quad = gluNewQuadric();

        glNewList(ce_render_system->axes_list, GL_COMPILE);
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

        glNewList(ce_render_system->wire_cube_list, GL_COMPILE);
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

        glNewList(ce_render_system->solid_cube_list, GL_COMPILE);
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

        glNewList(ce_render_system->solid_sphere_list, GL_COMPILE);
        gluSphere(quad, 1.0, 40, 40);
        glEndList();

        gluDeleteQuadric(quad);
    }

    void ce_render_system_term(void)
    {
        if (NULL != ce_render_system) {
            glDeleteLists(ce_render_system->solid_sphere_list, 1);
            glDeleteLists(ce_render_system->solid_cube_list, 1);
            glDeleteLists(ce_render_system->wire_cube_list, 1);
            glDeleteLists(ce_render_system->axes_list, 1);
            ce_free(ce_render_system, sizeof(struct ce_render_system));
        }
    }

    ce_thread_id ce_render_system_thread_id(void)
    {
        return ce_render_system->thread_id;
    }

    void ce_render_system_begin_render(const ce_color* clear_color)
    {
        glClearColor(clear_color->r, clear_color->g, clear_color->b, clear_color->a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
    }

    void ce_render_system_end_render(void)
    {
    #if 0
        if (FEATURE_MEMINFO) {
            const int n = 3;
            GLint params[4], viewport[4];
            GLenum pnames[] = { GL_VBO_FREE_MEMORY,
                                GL_TEXTURE_FREE_MEMORY,
                                GL_RENDERBUFFER_FREE_MEMORY };
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
        if (flag && PERFORMANCE_MONITOR) {
            flag = false;
            GLint group_count;
            gl_get_perfmon_groups(&group_count, 0, NULL);

            GLuint groups[group_count];
            gl_get_perfmon_groups(NULL, group_count, groups);

            for (int i = 0; i < group_count; ++i) {
                GLsizei length;
                gl_get_perfmon_group_str(groups[i], 0, &length, NULL);

                char group_name[length + 1];
                gl_get_perfmon_group_str(groups[i],
                    sizeof(group_name), NULL, group_name);

                GLint counter_count, max_active;
                gl_get_perfmon_counters(groups[i], &counter_count,
                                            &max_active, 0, NULL);

                GLuint counters[counter_count];
                gl_get_perfmon_counters(groups[i], NULL, NULL,
                                            counter_count, counters);

                printf("group: %s, max active: %d\n", group_name, max_active);

                for (int j = 0; j < counter_count; ++j) {
                    GLsizei length;
                    gl_get_perfmon_counter_str(groups[i],
                        counters[j], 0, &length, NULL);

                    char counter_name[length + 1];
                    gl_get_perfmon_counter_str(groups[i], counters[j],
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

    void ce_render_system_draw_axes(void)
    {
        glCallList(ce_render_system->axes_list);
    }

    void ce_render_system_draw_wire_cube(void)
    {
        glCallList(ce_render_system->wire_cube_list);
    }

    void ce_render_system_draw_solid_cube(void)
    {
        glCallList(ce_render_system->solid_cube_list);
    }

    void ce_render_system_draw_solid_sphere(void)
    {
        glCallList(ce_render_system->solid_sphere_list);
    }

    void ce_render_system_draw_fullscreen_wire_rect(unsigned int width, unsigned int height)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, width, 0, height);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2i(0, 0);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2i(width, 0);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2i(width, height);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2i(0, height);

        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    void ce_render_system_setup_viewport(ce_viewport* viewport)
    {
        glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
        glScissor(viewport->x, viewport->y, viewport->width, viewport->height);
    }

    void ce_render_system_setup_camera(ce_camera* camera)
    {
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

        ce_render_system->view.m[0] = 1.0f - (tyy + tzz);
        ce_render_system->view.m[1] = txy + twz;
        ce_render_system->view.m[2] = txz - twy;

        ce_render_system->view.m[4] = txy - twz;
        ce_render_system->view.m[5] = 1.0f - (txx + tzz);
        ce_render_system->view.m[6] = tyz + twx;

        ce_render_system->view.m[8] = txz + twy;
        ce_render_system->view.m[9] = tyz - twx;
        ce_render_system->view.m[10] = 1.0f - (txx + tyy);

        ce_render_system->view.m[12] = -
            ce_render_system->view.m[0] * camera->position.x -
            ce_render_system->view.m[4] * camera->position.y -
            ce_render_system->view.m[8] * camera->position.z;

        ce_render_system->view.m[13] = -
            ce_render_system->view.m[1] * camera->position.x -
            ce_render_system->view.m[5] * camera->position.y -
            ce_render_system->view.m[9] * camera->position.z;

        ce_render_system->view.m[14] = -
            ce_render_system->view.m[2] * camera->position.x -
            ce_render_system->view.m[6] * camera->position.y -
            ce_render_system->view.m[10] * camera->position.z;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(camera->fov, camera->aspect, camera->near, camera->far);

        glMatrixMode(GL_MODELVIEW);
        glMultMatrixf(ce_render_system->view.m);
    }

    void ce_render_system_apply_color(const ce_color* color)
    {
        glColor4f(color->r, color->g, color->b, color->a);
    }

    void ce_render_system_apply_transform(const ce_vec3* translation, const ce_quat* rotation, const ce_vec3* scaling)
    {
        ce_vec3 axis;
        float angle = ce_quat_to_polar(rotation, &axis);

        glPushMatrix();

        glTranslatef(translation->x, translation->y, translation->z);
        glRotatef(ce_rad2deg(angle), axis.x, axis.y, axis.z);
        glScalef(scaling->x, scaling->y, scaling->z);
    }

    void ce_render_system_discard_transform(void)
    {
        glPopMatrix();
    }

    void ce_render_system_apply_material(ce_material* material)
    {
        if (NULL != material->shader) {
            ce_shader_bind(material->shader);
        } else {
            glPushAttrib(GL_ENABLE_BIT);
            glEnable(GL_LIGHTING);

            const GLfloat material_ambient[] = { material->ambient.r, material->ambient.g, material->ambient.b, material->ambient.a };
            const GLfloat material_diffuse[] = { material->diffuse.r, material->diffuse.g, material->diffuse.b, material->diffuse.a };
            const GLfloat material_specular[] = { material->specular.r, material->specular.g, material->specular.b, material->specular.a };
            const GLfloat material_emission[] = { material->emission.r, material->emission.g, material->emission.b, material->emission.a };

            glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
            glMaterialfv(GL_FRONT, GL_EMISSION, material_emission);
            glMaterialf(GL_FRONT, GL_SHININESS, material->shininess);

            const GLint material_modes[] = { GL_MODULATE, GL_DECAL, GL_REPLACE };
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, material_modes[material->mode]);

            if (material->alpha_test) {
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GREATER, 0.5f);
            }

            if (material->blend) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }
    }

    void ce_render_system_discard_material(ce_material* material)
    {
        if (NULL != material->shader) {
            ce_shader_unbind(material->shader);
        } else {
            glPopAttrib();
        }
    }
}
