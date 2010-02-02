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

#ifndef CE_IO_H
#define CE_IO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	int (*close)(void* client_data);
	size_t (*read)(void* client_data, void* data, size_t size, size_t n);
	size_t (*write)(void* client_data, const void* data, size_t size, size_t n);
	int (*seek)(void* client_data, long int offset, int whence);
	long int (*tell)(void* client_data);
} ce_io_callbacks;

extern const ce_io_callbacks CE_IO_CALLBACKS_FILE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_IO_H */
