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

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "celogging.h"
#include "ceerror.h"

void ce_error_report_c_last(const char* module)
{
	ce_error_report_c_errno(errno, module);
}

void ce_error_report_c_errno(int code, const char* module)
{
#ifdef _POSIX_C_SOURCE
	for (size_t length = 16, limit = 512; length <= limit; length <<= 1) {
		char buffer[length];
		switch (strerror_r(code, buffer, length)) {
		case 0:
			ce_logging_error("%s: %s", module, buffer);
			return;
		case ERANGE:
			// try again
			break;
		case EINVAL:
			ce_logging_critical("error: %d is not a valid error number", code);
		default:
			return;
		}
	}
#else
	// FIXME: strerror is not required to be reentrant
	ce_logging_error("%s: %s", module, strerror(code));
#endif
}
