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

#ifndef CE_EVENT_H
#define CE_EVENT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ce_event {
	void (*notify)(struct ce_event* event);
	size_t size;
	char impl[];
} ce_event;

extern ce_event* ce_event_new(void (*notify)(ce_event*), size_t size);
extern void ce_event_del(ce_event* event);

typedef struct {
	void* ptr;
} ce_event_ptr;

#ifdef __cplusplus
}
#endif

#endif /* CE_EVENT_H */
