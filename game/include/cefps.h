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

#ifndef CE_FPS_H
#define CE_FPS_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_fps ce_fps;

extern ce_fps* ce_fps_new(void);
extern void ce_fps_del(ce_fps* fps);

extern void ce_fps_advance(ce_fps* fps, float elapsed);
extern void ce_fps_render(ce_fps* fps);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FPS_H */
