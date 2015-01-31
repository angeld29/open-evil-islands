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

#ifndef CE_MPRMANAGER_HPP
#define CE_MPRMANAGER_HPP

#include "string.hpp"
#include "mprfile.hpp"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_mpr_manager {
    int stub;
}* ce_mpr_manager;

extern void ce_mpr_manager_init(void);
extern void ce_mpr_manager_term(void);

extern ce_mprfile* ce_mpr_manager_open(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* CE_MPRMANAGER_HPP */
