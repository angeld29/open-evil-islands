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

#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "path.hpp"

namespace cursedearth
{
const char CE_PATH_SEP = '/';

bool ce_path_exists(const char* path)
{
    struct stat info;
    return 0 == stat(path, &info);
}

bool ce_path_is_dir(const char* path)
{
    struct stat info;
    return 0 == stat(path, &info) && S_ISDIR(info.st_mode);
}

bool ce_path_is_file(const char* path)
{
    struct stat info;
    return 0 == stat(path, &info) && S_ISREG(info.st_mode);
}

bool ce_path_list_subdirs(const char* path, ce_vector* subdirs)
{
    DIR* dir;
    struct dirent* entry;
    char buffer[CE_PATH_MAX];

    if (NULL == (dir = opendir(path))) {
        return false;
    }

    while (NULL != (entry = readdir(dir))) {
        if (0 != strcmp(".", entry->d_name) && 0 != strcmp("..", entry->d_name)) {
            ce_path_join(buffer, sizeof(buffer), path, entry->d_name, NULL);
            if (ce_path_is_dir(buffer)) {
                ce_vector_push_back(subdirs, ce_string_new_str(buffer));
            }
        }
    }

    closedir(dir);
    return true;
}
}
