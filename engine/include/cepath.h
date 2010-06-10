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

#ifndef CE_PATH_H
#define CE_PATH_H

#include <stdbool.h>

#include "cestring.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const char CE_PATH_SEP;

/*
 *  See also python os.path module.
*/

extern bool ce_path_exists(const char* path);
extern void ce_path_join(ce_string* path1, const char* path2);
extern void ce_path_normpath(ce_string* path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_PATH_H */
