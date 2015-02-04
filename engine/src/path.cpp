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
#include <cstring>
#include <vector>

#include "lib.hpp"
#include "str.hpp"
#include "path.hpp"

namespace cursedearth
{
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
            length = ce_min(size_t, length, size - 1);
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

    char* ce_path_normpath(char* path)
    {
        for (char* sep = strpbrk(path, "/\\"); NULL != sep; sep = strpbrk(sep + 1, "/\\")) {
            *sep = CE_PATH_SEP;
        }
        return path;
    }

    char* ce_path_append_ext(char* file_name, size_t size, const char* name, const char* ext)
    {
        if (NULL == ce_strcasestr(name, ext)) {
            snprintf(file_name, size, "%s%s", name, ext);
        } else {
            ce_strlcpy(file_name, name, size);
        }
        return file_name;
    }

    char* ce_path_remove_ext(char* name, const char* file_name)
    {
        const char* dot = strrchr(file_name, '.');
        size_t n = NULL != dot ? (size_t)(dot - file_name) : strlen(file_name);
        return ce_strleft(name, file_name, n);
    }

    char* ce_path_find_special1(char* path, size_t size, const char* prefix, const char* name, const char* dirs[], const char* exts[])
    {
        for (size_t i = 0; NULL != exts[i]; ++i) {
            std::vector<char> file_name(strlen(name) + strlen(exts[i]) + 1);
            ce_path_append_ext(file_name.data(), file_name.size(), name, exts[i]);
            for (size_t j = 0; NULL != dirs[j]; ++j) {
                if (NULL != ce_path_join(path, size, prefix, dirs[j], file_name.data(), NULL) && ce_path_exists(path)) {
                    return path;
                }
            }
        }
        return NULL;
    }
}
