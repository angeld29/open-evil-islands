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

#include <windows.h>

#include "exception.hpp"
#include "logging.hpp"
#include "systeminfo.hpp"

namespace cursedearth
{
    void detect_system()
    {
        OSVERSIONINFOEX osverinfo;
        ZeroMemory(&osverinfo, sizeof(OSVERSIONINFOEX));
        osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        BOOL osverinfoex = GetVersionEx((OSVERSIONINFO*)&osverinfo);
        if (!osverinfoex) {
            ZeroMemory(&osverinfo, sizeof(OSVERSIONINFOEX));
            osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (!GetVersionEx((OSVERSIONINFO*)&osverinfo)) {
                throw game_error("system info", "unable to get os version info");
            }
        }

        if (VER_PLATFORM_WIN32_NT != osverinfo.dwPlatformId) {
            throw game_error("system info", "Microsoft Windows NT platform required");
        }

        std::string message;
        if (6 == osverinfo.dwMajorVersion && 3 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows 8.1 ";
        } else if (6 == osverinfo.dwMajorVersion && 2 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows 8 ";
        } else if (6 == osverinfo.dwMajorVersion && 1 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows 7 ";
        } else if (6 == osverinfo.dwMajorVersion && 0 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows Vista ";
        } else if (5 == osverinfo.dwMajorVersion && 2 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows XP Professional ";
        } else if (5 == osverinfo.dwMajorVersion && 1 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows XP ";
        } else if (5 == osverinfo.dwMajorVersion && 0 == osverinfo.dwMinorVersion) {
            message = "Microsoft Windows 2000 ";
        } else {
            message = "Microsoft Windows NT ";
        }

        if (osverinfoex) {
            if (VER_NT_WORKSTATION == osverinfo.wProductType) {
                message += (osverinfo.wSuiteMask & VER_SUITE_PERSONAL) ? "Personal " : "Professional ";
            } else {
                message += "Server ";
            }
        }

        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);

        if (PROCESSOR_ARCHITECTURE_AMD64 == sysinfo.wProcessorArchitecture) {
            message += "x86_64 ";
        } else if (PROCESSOR_ARCHITECTURE_INTEL == sysinfo.wProcessorArchitecture) {
            message += "x86 ";
        } else if (PROCESSOR_ARCHITECTURE_IA64 == sysinfo.wProcessorArchitecture) {
            message += "Intel Itanium ";
        }

        message += str(boost::format("Version %1%.%2% %3% Build %4%") % osverinfo.dwMajorVersion %
            osverinfo.dwMinorVersion % osverinfo.szCSDVersion % (osverinfo.dwBuildNumber & 0xffff));

        ce_logging_info("system info: %s", message.c_str());

        if (osverinfo.dwMajorVersion < 5) {
            throw game_error("system info", "Windows XP Professional or above required");
        }

        ce_logging_info("system info: Windows XP Professional or above detected");
    }
}
