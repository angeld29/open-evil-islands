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

#ifndef CE_PATH_HPP
#define CE_PATH_HPP

#include <cstddef>
#include <cstdarg>

#include "string.hpp"
#include "vector.hpp"

namespace cursedearth
{
    enum {
        CE_PATH_MAX = 512,
    };

    extern const char CE_PATH_SEP;

    extern bool ce_path_exists(const char* path);

    extern bool ce_path_is_dir(const char* path);
    extern bool ce_path_is_file(const char* path);

    extern bool ce_path_list_subdirs(const char* path, ce_vector* subdirs);

    extern char* ce_path_join_va(char* path, size_t size, va_list args);
    extern char* ce_path_join(char* path, size_t size, ...);

    extern char* ce_path_normpath(char* path);

    extern char* ce_path_append_ext(char* file_name, size_t size, const char* name, const char* ext);
    extern char* ce_path_remove_ext(char* name, const char* file_name);

    extern char* ce_path_find_special1(char* path, size_t size, const char* prefix, const char* name, const char* dirs[], const char* exts[]);
}

#endif
