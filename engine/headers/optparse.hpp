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

#ifndef CE_OPTPARSE_HPP
#define CE_OPTPARSE_HPP

#include <memory>

#include "vector.hpp"
#include "string.hpp"
#include "object.hpp"

namespace cursedearth
{
    struct ce_optparse
    {
        ce_optparse();
        ~ce_optparse();

        int version_major;
        int version_minor;
        int version_patch;
        ce_string* title;
        ce_string* brief;
        ce_vector* argobjects;
        ce_vector* ctrlobjects;
    };

    typedef std::shared_ptr<ce_optparse> ce_optparse_ptr_t;

    void ce_optparse_set_standard_properties(const ce_optparse_ptr_t& optparse, int version_major, int version_minor, int version_patch, const char* title, const char* brief);

    bool ce_optparse_get(const ce_optparse_ptr_t& optparse, const char* name, void* value);

    void ce_optparse_add(const ce_optparse_ptr_t& optparse, const char* name, ce_type type, const void* value, bool required, const char* shortopt, const char* longopt, const char* glossary);
    void ce_optparse_add_control(const ce_optparse_ptr_t& optparse, const char* name, const char* glossary);

    bool ce_optparse_parse(const ce_optparse_ptr_t& optparse, int argc, char* argv[]);
}

#endif
