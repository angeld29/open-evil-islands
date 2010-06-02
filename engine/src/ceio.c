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

#include <stdio.h>

#include "ceio.h"

static int ce_io_file_close(void* client_data)
{
	return fclose(client_data);
}

static size_t ce_io_file_read(void* client_data, void* data, size_t size, size_t n)
{
	return fread(data, size, n, client_data);
}

static int ce_io_file_seek(void* client_data, long int offset, int whence)
{
	return fseek(client_data, offset, whence);
}

static long int ce_io_file_tell(void* client_data)
{
	return ftell(client_data);
}

const ce_io_callbacks CE_IO_CALLBACKS_FILE = {
	ce_io_file_close, ce_io_file_read, ce_io_file_seek, ce_io_file_tell
};
