/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <string>

#include "mprfile.hpp"

namespace cursedearth
{
    extern struct ce_mpr_manager {
        int stub;
    }* ce_mpr_manager;

    void ce_mpr_manager_init();
    void ce_mpr_manager_term();

    ce_mprfile* ce_mpr_manager_open(const std::string& name);
}

#endif
