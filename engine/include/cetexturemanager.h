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

#ifndef CE_TEXTUREMANAGER_H
#define CE_TEXTUREMANAGER_H

#include <stdbool.h>

#include "cestring.h"
#include "cevector.h"
#include "cemmpfile.h"
#include "cetexture.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_texture_manager {
    ce_vector* res_files;
    ce_vector* textures;
}* ce_texture_manager;

extern void ce_texture_manager_init(void);
extern void ce_texture_manager_term(void);

// search mmp file only in cache directory; thread-safe
extern ce_mmpfile* ce_texture_manager_open_mmpfile_from_cache(const char* name);

// search mmp file only in resources; not thread-safe
extern ce_mmpfile* ce_texture_manager_open_mmpfile_from_resources(const char* name);

// search mmp file in both cache directory and resources
extern ce_mmpfile* ce_texture_manager_open_mmpfile(const char* name);

// save mmp file in cache directory; thread-safe
extern void ce_texture_manager_save_mmpfile(const char* name, ce_mmpfile* mmpfile);

// acquire texture, not thread-safe
extern ce_texture* ce_texture_manager_get(const char* name);

// add new texture, not thread-safe
extern void ce_texture_manager_put(ce_texture* texture);

#ifdef __cplusplus
}
#endif

#endif /* CE_TEXTUREMANAGER_H */
