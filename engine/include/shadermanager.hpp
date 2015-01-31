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

#ifndef CE_SHADERMANAGER_HPP
#define CE_SHADERMANAGER_HPP

#include "vector.hpp"
#include "shader.hpp"

namespace cursedearth
{
    extern struct ce_shader_manager {
        ce_vector* shaders;
    }* ce_shader_manager;

    extern void ce_shader_manager_init(void);
    extern void ce_shader_manager_term(void);

    extern ce_shader* ce_shader_manager_get(const char* resource_paths[]);
}

#endif /* CE_SHADERMANAGER_HPP */
