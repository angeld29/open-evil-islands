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

#include <stdio.h>
#include <assert.h>

#include <windows.h>

#include "cestr.h"
#include "celogging.h"
#include "cesysteminfo.h"

bool ce_system_info_check(void)
{
    OSVERSIONINFOEX osverinfo;
    BOOL osverinfoex;

    ZeroMemory(&osverinfo, sizeof(OSVERSIONINFOEX));
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    osverinfoex = GetVersionEx((OSVERSIONINFO*)&osverinfo);
    if (!osverinfoex) {
        osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!GetVersionEx((OSVERSIONINFO*)&osverinfo)) {
            ce_logging_fatal("system info: could not retrieve os version info");
            return false;
        }
    }

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    char buffer[512], tmp[128];

    switch (osverinfo.dwPlatformId) {
    case VER_PLATFORM_WIN32s:
    case VER_PLATFORM_WIN32_WINDOWS:
        ce_logging_write("system info: Microsoft Windows %lu.%lu %s",
            osverinfo.dwMajorVersion, osverinfo.dwMinorVersion, osverinfo.szCSDVersion);
        break;

    case VER_PLATFORM_WIN32_NT:
        if (6 == osverinfo.dwMajorVersion && 1 == osverinfo.dwMinorVersion) {
            ce_strlcpy(buffer, "Microsoft Windows 7 ", sizeof(buffer));
        } else if (6 == osverinfo.dwMajorVersion && 0 == osverinfo.dwMinorVersion) {
            ce_strlcpy(buffer, "Microsoft Windows Vista ", sizeof(buffer));
        } else if (5 == osverinfo.dwMajorVersion && 1 == osverinfo.dwMinorVersion) {
            ce_strlcpy(buffer, "Microsoft Windows XP ", sizeof(buffer));
        } else if (5 == osverinfo.dwMajorVersion && 0 == osverinfo.dwMinorVersion) {
            ce_strlcpy(buffer, "Microsoft Windows 2000 ", sizeof(buffer));
        } else {
            ce_strlcpy(buffer, "Microsoft Windows NT ", sizeof(buffer));
        }

        if (osverinfoex) {
            if (VER_NT_WORKSTATION == osverinfo.wProductType) {
                if (osverinfo.wSuiteMask & VER_SUITE_PERSONAL) {
                    ce_strlcat(buffer, "Personal ", sizeof(buffer));
                } else {
                    ce_strlcat(buffer, "Professional ", sizeof(buffer));
                }
            } else {
                if (osverinfo.wSuiteMask & VER_SUITE_DATACENTER) {
                    ce_strlcat(buffer, "DataCenter Server ", sizeof(buffer));
                } else if (osverinfo.wSuiteMask & VER_SUITE_ENTERPRISE) {
                    ce_strlcat(buffer, "Advanced Server ", sizeof(buffer));
                } else {
                    ce_strlcat(buffer, "Server ", sizeof(buffer));
                }
            }
        }

        if (PROCESSOR_ARCHITECTURE_AMD64 == sysinfo.wProcessorArchitecture) {
            ce_strlcat(buffer, "x86_64 ", sizeof(buffer));
        }

        snprintf(tmp, sizeof(tmp), "Version %lu.%lu %s Build %lu",
            osverinfo.dwMajorVersion, osverinfo.dwMinorVersion,
            osverinfo.szCSDVersion, osverinfo.dwBuildNumber & 0xffff);
        ce_strlcat(buffer, tmp, sizeof(buffer));

        ce_logging_write("system info: %s", buffer);
        break;
    }

    if (VER_PLATFORM_WIN32_NT != osverinfo.dwPlatformId ||
            osverinfo.dwMajorVersion < 5 || osverinfo.dwMinorVersion < 1) {
        ce_logging_fatal("system info: Windows XP or above required");
        return false;
    }

    ce_logging_write("system info: Windows XP or above detected");
    return true;
}
