/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#ifndef CE_RESOURCEMANAGER_H
#define CE_RESOURCEMANAGER_H

#include "cestring.h"
#include "ceresourcedata.h"
#include "ceresfile.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_resource_manager {
    ce_string* path;
    ce_res_file* database;
    ce_res_file* menus;
}* ce_resource_manager;

extern void ce_resource_manager_init(void);
extern void ce_resource_manager_term(void);

extern size_t ce_resource_manager_find_data(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* CE_RESOURCEMANAGER_H */
