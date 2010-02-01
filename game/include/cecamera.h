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

#ifndef CE_CAMERA_H
#define CE_CAMERA_H

#include "vec3fwd.h"
#include "quatfwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_camera ce_camera;

extern ce_camera* ce_camera_open(void);
extern void ce_camera_close(ce_camera* cam);

extern float ce_camera_get_fov(ce_camera* cam);
extern float ce_camera_get_aspect(ce_camera* cam);
extern float ce_camera_get_near(ce_camera* cam);
extern float ce_camera_get_far(ce_camera* cam);

extern vec3* ce_camera_get_eye(vec3* eye, ce_camera* cam);
extern vec3* ce_camera_get_forward(vec3* forward, ce_camera* cam);
extern vec3* ce_camera_get_up(vec3* up, ce_camera* cam);
extern vec3* ce_camera_get_right(vec3* right, ce_camera* cam);

extern void ce_camera_set_fov(float fov, ce_camera* cam);
extern void ce_camera_set_aspect(int width, int height, ce_camera* cam);
extern void ce_camera_set_near(float near, ce_camera* cam);
extern void ce_camera_set_far(float far, ce_camera* cam);

extern void ce_camera_set_eye(const vec3* eye, ce_camera* cam);
extern void ce_camera_set_look(const quat* look, ce_camera* cam);

extern void ce_camera_move(float offset_x, float offset_z, ce_camera* cam);
extern void ce_camera_zoom(float offset, ce_camera* cam);
extern void ce_camera_yaw_pitch(float psi, float theta, ce_camera* cam);

extern void ce_camera_setup(ce_camera* cam);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CAMERA_H */
