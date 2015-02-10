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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

#include <boost/filesystem.hpp>

#include "str.hpp"
#include "path.hpp"

namespace cursedearth
{
    char* ce_path_remove_ext(char* name, const char* file_name)
    {
        const char* dot = strrchr(file_name, '.');
        size_t n = NULL != dot ? (size_t)(dot - file_name) : strlen(file_name);
        return ce_strleft(name, file_name, n);
    }

    char* ce_path_join_va(char* path, size_t size, va_list args)
    {
        if (0 == size) {
            return NULL;
        }
        path[0] = '\0';
        size_t length = 0;
        const char sep[2] = {CE_PATH_SEP, '\0'};
        for (const char* tail = va_arg(args, const char*); NULL != tail; tail = va_arg(args, const char*)) {
            if (0 != length && '/' != path[length - 1] && '\\' != path[length - 1]) {
                ce_strlcat(path, sep, size);
                ++length;
            }
            ce_strlcat(path, tail, size);
            length += strlen(tail);
            length = std::min(length, size - 1);
        }
        return path;
    }

    char* ce_path_join(char* path, size_t size, ...)
    {
        va_list args;
        va_start(args, size);
        path = ce_path_join_va(path, size, args);
        va_end(args);
        return path;
    }

    char* ce_path_find_special1(char* path, size_t size, const char* prefix, const char* name, const char* dirs[], const char* exts[])
    {
        for (size_t i = 0; NULL != exts[i]; ++i) {
            std::string file_name = std::string(name) + exts[i];
            for (size_t j = 0; NULL != dirs[j]; ++j) {
                if (NULL != ce_path_join(path, size, prefix, dirs[j], file_name.c_str(), NULL) && boost::filesystem::exists(path)) {
                    return path;
                }
            }
        }
        return NULL;
    }
}
