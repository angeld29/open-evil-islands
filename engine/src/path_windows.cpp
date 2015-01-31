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

#include <assert.h>

#include <windows.h>

#include "path.hpp"

const char CE_PATH_SEP = '\\';

bool ce_path_exists(const char* path)
{
    return INVALID_FILE_ATTRIBUTES != GetFileAttributes(path);
}

bool ce_path_is_dir(const char* CE_UNUSED(path))
{
    assert(false && "not implemented");
    return false;
}

bool ce_path_is_file(const char* CE_UNUSED(path))
{
    assert(false && "not implemented");
    return false;
}

bool ce_path_list_subdirs(const char* CE_UNUSED(path), ce_vector* CE_UNUSED(subdirs))
{
    assert(false && "not implemented");
    return false;
}
