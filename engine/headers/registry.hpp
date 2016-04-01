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

#ifndef CE_REGISTRY_HPP
#define CE_REGISTRY_HPP

#include <string>

#include <boost/filesystem/path.hpp>

namespace cursedearth
{
    enum class registry_key_t {
        current_user,
        local_machine
    };

    std::string find_string_in_registry(registry_key_t, const std::string& key_name, const std::string& value_name);
    boost::filesystem::path find_path_in_registry(registry_key_t, const std::string& key_name, const std::string& value_name);
}

#endif
