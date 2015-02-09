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

#include <cstdlib>
#include <memory>
#include <functional>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <boost/algorithm/string.hpp>

#include "logging.hpp"
#include "configfile.hpp"
#include "registry.hpp"

namespace cursedearth
{
    namespace
    {
        bool is_directory(const std::string& path)
        {
            struct stat info;
            return 0 == stat(path.c_str(), &info) && S_ISDIR(info.st_mode);
        }

        std::vector<std::string> list_subdirectories(const std::string& path)
        {
            std::vector<std::string> directories;
            if (DIR* dir = opendir(path.c_str())) {
                struct dirent* entry;
                while ((entry = readdir(dir))) {
                    std::string node_name(entry->d_name);
                    if ("." != node_name && ".." != node_name) {
                        std::string directory = path + '/' + node_name;
                        if (is_directory(directory)) {
                            directories.push_back(directory);
                        }
                    }
                }
                closedir(dir);
            }
            return directories;
        }

        std::string get_home_path()
        {
            if (const char* home_path = getenv("HOME")) {
                return home_path;
            }
            throw std::runtime_error("could not get environment variable `HOME'");
        }

        std::string get_reg_file_name(registry_key_t key)
        {
            switch (key) {
            case registry_key_t::current_user:  return "user.reg";
            case registry_key_t::local_machine: return "system.reg";
            }
            throw std::runtime_error("unknown key");
        }

        struct config_file_deleter_t
        {
            void operator ()(ce_config_file* config_file)
            {
                ce_config_file_close(config_file);
            }
        };

        std::string get_value(const std::string& wine_prefix, const std::string& reg_file_name, const std::string& key_name, const std::string& value_name)
        {
            const std::string reg_file_path = wine_prefix + '/' + reg_file_name;
            std::unique_ptr<ce_config_file, config_file_deleter_t> config_file(ce_config_file_open(reg_file_path.c_str()));
            if (!config_file) {
                ce_logging_error("registry: could not open file `%s'", reg_file_path.c_str());
                return std::string();
            }

            // double backslash
            const std::string fixed_key_name = boost::replace_all_copy(key_name, "\\", "\\\\");

            // append quotes
            const std::string fixed_value_name = '"' + value_name + '"';

            std::string reg_value = ce_config_file_find(config_file.get(), fixed_key_name.c_str(), fixed_value_name.c_str());
            if (reg_value.empty()) {
                ce_logging_warning("registry: could not find `%s\\%s'", key_name.c_str(), value_name.c_str());
                return std::string();
            }

            // remove quotes
            boost::algorithm::replace_first(reg_value, "\"", "");
            boost::algorithm::replace_last(reg_value, "\"", "");

            return reg_value;
        }

        std::string get_string(const std::vector<std::string>& wine_prefixes, const std::string& reg_file_name, const std::string& key_name, const std::string& value_name)
        {
            for (const auto& wine_prefix: wine_prefixes) {
                const std::string value = get_value(wine_prefix, reg_file_name, key_name, value_name);
                if (!value.empty()) {
                    return value;
                }
            }
            return std::string();
        }

        std::string get_path(const std::vector<std::string>& wine_prefixes, const std::string& reg_file_name, const std::string& key_name, const std::string& value_name)
        {
            for (const auto& wine_prefix: wine_prefixes) {
                std::string value = get_value(wine_prefix, reg_file_name, key_name, value_name);
                if (!value.empty()) {
                    boost::replace_all(value, "\\\\", "/");
                    boost::ireplace_all(value, "c:", "drive_c");
                    boost::ireplace_all(value, "d:", "drive_d");
                    // more?
                    return wine_prefix + '/' + value;
                }
            }
            return std::string();
        }

        typedef std::function<std::string (const std::vector<std::string>&, const std::string&, const std::string&, const std::string&)> get_func_t;

        std::string find_value(registry_key_t key, const std::string& key_name, const std::string& value_name, const get_func_t& func)
        {
            try {
                const std::string home_path = get_home_path();
                const std::string reg_file_name = get_reg_file_name(key);

                // add .PlayOnLinux prefixes (preferred)
                std::vector<std::string> wine_prefixes = list_subdirectories(home_path + "/.PlayOnLinux/wineprefix");

                // add .wine prefix at the end
                wine_prefixes.push_back(home_path + "/.wine");

                return func(wine_prefixes, reg_file_name, key_name, value_name);
            } catch (const std::exception& error) {
                ce_logging_error("registry: %s", error.what());
            }

            // not critical error, just return empty string
            return std::string();
        }
    }

    std::string find_string_in_registry(registry_key_t key, const std::string& key_name, const std::string& value_name)
    {
        return find_value(key, key_name, value_name, get_string);
    }

    std::string find_path_in_registry(registry_key_t key, const std::string& key_name, const std::string& value_name)
    {
        return find_value(key, key_name, value_name, get_path);
    }
}
