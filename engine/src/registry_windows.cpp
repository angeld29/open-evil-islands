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

#include <memory>

#include <windows.h>

#include <boost/algorithm/string.hpp>

#include "registry.hpp"

namespace cursedearth
{
    struct hkey_dtor_t
    {
        void operator ()(HKEY* hkey)
        {
            RegCloseKey(*hkey);
        }
    };

    std::string find_string_in_registry(registry_key_t key, const std::string& key_name, const std::string& value_name)
    {
        HKEY hkey;
        DWORD htype = REG_SZ;

        BYTE data[1024];
        DWORD size = sizeof(data);

        switch (key) {
        case registry_key_t::current_user:
            hkey = HKEY_CURRENT_USER;
            break;
        case registry_key_t::local_machine:
            hkey = HKEY_LOCAL_MACHINE;
            break;
        default:
            return std::string();
        }

        if (ERROR_SUCCESS == RegOpenKeyEx(hkey, key_name.c_str(), 0, KEY_READ, &hkey)) {
            std::unique_ptr<HKEY, hkey_dtor_t> hhkey(&hkey);
            if (ERROR_SUCCESS == RegQueryValueEx(*hhkey, value_name.c_str(), NULL, &htype, data, &size)) {
                if (REG_SZ == htype) {
                    // size includes any terminating null character or characters unless the data was stored without them
                    return boost::algorithm::trim_right_copy_if(std::string(reinterpret_cast<char*>(data), size), [] (char c) { return '\0' == c; });
                }
            }
        }

        return std::string();
    }

    std::string find_path_in_registry(registry_key_t key, const std::string& key_name, const std::string& value_name)
    {
        return find_string_in_registry(key, key_name, value_name);
    }
}
