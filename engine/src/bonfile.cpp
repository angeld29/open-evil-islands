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

#include "alloc.hpp"
#include "bonfile.hpp"

namespace cursedearth
{
    ce_bonfile* ce_bonfile_open(ce_res_file* res_file, const char* name)
    {
        size_t index = ce_res_file_node_index(res_file, name);
        ce_bonfile* bonfile = (ce_bonfile*)ce_alloc(sizeof(ce_bonfile));
        bonfile->size = ce_res_file_node_size(res_file, index);
        bonfile->bone = (float*)ce_res_file_node_data(res_file, index);
        return bonfile;
    }

    void ce_bonfile_close(ce_bonfile* bonfile)
    {
        if (NULL != bonfile) {
            ce_free(bonfile->bone, bonfile->size);
            ce_free(bonfile, sizeof(ce_bonfile));
        }
    }
}
