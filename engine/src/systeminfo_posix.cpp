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

#include <stdexcept>

#include <sys/utsname.h>
#include <unistd.h>

#include "logging.hpp"
#include "systeminfo.hpp"

namespace cursedearth
{
    void detect_system()
    {
        struct utsname osinfo;
        if (uname(&osinfo) >= 0) {
            ce_logging_info("system info: %s %s %s %s %s", osinfo.sysname, osinfo.nodename, osinfo.release, osinfo.version, osinfo.machine);
        }

        const std::string posix_2004_v3 = "SUSv3 POSIX.1-2004 (IEEE Std 1003.1-2004) system";
        const std::string posix_2008_v4 = "SUSv4 POSIX.1-2008 (IEEE Std 1003.1-2008) system";

        // _SC_VERSION inquire about the parameter corresponding to _POSIX_VERSION
        const long posix_version = sysconf(_SC_VERSION);

        // _SC_XOPEN_VERSION inquire about the parameter corresponding to _XOPEN_VERSION
        const long xopen_version = sysconf(_SC_XOPEN_VERSION);

        if (posix_version < 200112l || xopen_version < 600l) {
            throw std::runtime_error(posix_2004_v3 + " or above required");
        }

        if (posix_version == 200809l && xopen_version == 700l) {
            ce_logging_info("system info: %s detected", posix_2008_v4.c_str());
        } else {
            ce_logging_info("system info: %s or above detected", posix_2004_v3.c_str());
        }
    }
}
