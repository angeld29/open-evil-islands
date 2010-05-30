/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include <assert.h>

#include <sys/utsname.h>
#include <unistd.h>

#include "celogging.h"
#include "cesysteminfo.h"

void ce_systeminfo_display(void)
{
	struct utsname osinfo;
	uname(&osinfo);

	ce_logging_write("systeminfo: %s %s %s %s %s", osinfo.sysname,
		osinfo.nodename, osinfo.release, osinfo.version, osinfo.machine);
}

bool ce_systeminfo_ensure(void)
{
	if (sysconf(_SC_VERSION) < 200112L || sysconf(_SC_XOPEN_VERSION) < 600L) {
		ce_logging_fatal("systeminfo: SUSv3 (POSIX.1-2001 + XPG6) system required");
		return false;
	}

	ce_logging_write("systeminfo: SUSv3 (POSIX.1-2001 + XPG6) system detected");
	return true;
}
