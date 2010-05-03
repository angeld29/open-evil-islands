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

#ifndef CE_ROOT_H
#define CE_ROOT_H

#include <stdbool.h>

#include "cerenderwindow.h"
#include "cerendersystem.h"
#include "cescenemng.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern struct ce_root {
	bool inited;
	ce_renderwindow* renderwindow;
	ce_rendersystem* rendersystem;
	ce_scenemng* scenemng;
} ce_root;

extern bool ce_root_init(const char* ei_path);
extern void ce_root_term(void);

extern void ce_root_exec(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC2_H */