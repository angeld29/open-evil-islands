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

#include <cassert>
#include <vector>

#include "alloc.hpp"
#include "logging.hpp"
#include "resourcemanager.hpp"
#include "opengl.hpp"
#include "shader.hpp"

namespace cursedearth
{
    typedef struct {
        GLuint program;
    } ce_shader_opengl;

    const GLenum ce_shader_opengl_types[CE_SHADER_TYPE_COUNT] = {
        [CE_SHADER_TYPE_UNKNOWN] = 0,
        [CE_SHADER_TYPE_VERTEX] = GL_VERTEX_SHADER,
        [CE_SHADER_TYPE_FRAGMENT] = GL_FRAGMENT_SHADER,
    };

    bool ce_shader_is_available(void)
    {
#ifdef CE_DISABLE_SHADERS
        return false;
#endif
        // programmable shaders were promoted to core features in OpenGL 2.0
        return GLEW_VERSION_2_0;
    }

    void ce_shader_report_errors(GLuint object)
    {
        GLint length;
        if (glIsShader(object)) {
            glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
        } else {
            glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
        }

        if (length > 0) {
            std::vector<char> buffer(length);
            if (glIsShader(object)) {
                glGetShaderInfoLog(object, length, NULL, buffer.data());
            } else {
                glGetProgramInfoLog(object, length, NULL, buffer.data());
            }
            ce_logging_error("shader: %s", buffer.data());
        } else {
            ce_logging_error("shader: unknown error");
        }
    }

    bool ce_shader_check_status(GLuint object)
    {
        GLint result;
        if (glIsShader(object)) {
            glGetShaderiv(object, GL_COMPILE_STATUS, &result);
        } else {
            glGetProgramiv(object, GL_LINK_STATUS, &result);
        }

        return 0 != result;
    }

    GLuint ce_shader_create_object(const ce_shader_info* shader_info)
    {
        GLuint object = glCreateShader(ce_shader_opengl_types[shader_info->shader_type]);

        const GLchar* data = reinterpret_cast<const GLchar*>(ce_resource_data[shader_info->resource_index]);
        const GLint data_size = ce_resource_data_sizes[shader_info->resource_index];

        glShaderSource(object, 1, &data, &data_size);
        glCompileShader(object);

        if (!ce_shader_check_status(object)) {
            ce_shader_report_errors(object);
            glDeleteShader(object);
            return 0;
        }

        return object;
    }

    ce_shader* ce_shader_new(const char* name, const ce_shader_info shader_infos[])
    {
        if (!ce_shader_is_available()) {
            return NULL;
        }

        ce_shader* shader = (ce_shader*)ce_alloc(sizeof(ce_shader));
        shader->impl = ce_alloc(sizeof(ce_shader_opengl));
        ce_shader_opengl* opengl_shader = (ce_shader_opengl*)shader->impl;

        shader->ref_count = 1;
        shader->name = ce_string_new_str(name);

        opengl_shader->program = glCreateProgram();

        for (size_t i = 0; CE_SHADER_TYPE_UNKNOWN != shader_infos[i].shader_type && shader_infos[i].resource_index < CE_RESOURCE_DATA_COUNT; ++i) {
            GLuint object = ce_shader_create_object(&shader_infos[i]);
            if (0 != object) {
                glAttachShader(opengl_shader->program, object);
                glDeleteShader(object);
            }
        }

        glLinkProgram(opengl_shader->program);

        if (!ce_shader_check_status(opengl_shader->program)) {
            ce_shader_report_errors(opengl_shader->program);
            ce_shader_del(shader);
            return NULL;
        }

        return shader;
    }

    void ce_shader_del(ce_shader* shader)
    {
        if (NULL != shader) {
            assert(ce_atomic_fetch(int, &shader->ref_count) > 0);
            if (0 == ce_atomic_dec_and_fetch(int, &shader->ref_count)) {
                ce_shader_opengl* opengl_shader = (ce_shader_opengl*)shader->impl;
                glDeleteProgram(opengl_shader->program);
                ce_string_del(shader->name);
                ce_free(shader->impl, sizeof(ce_shader_opengl));
                ce_free(shader, sizeof(ce_shader));
            }
        }
    }

    bool ce_shader_is_valid(const ce_shader* shader)
    {
        ce_shader_opengl* opengl_shader = (ce_shader_opengl*)shader->impl;
        return glIsProgram(opengl_shader->program);
    }

    void ce_shader_bind(ce_shader* shader)
    {
        ce_shader_opengl* opengl_shader = (ce_shader_opengl*)shader->impl;
        glUseProgram(opengl_shader->program);
    }

    void ce_shader_unbind(ce_shader*)
    {
        glUseProgram(0);
    }
}
