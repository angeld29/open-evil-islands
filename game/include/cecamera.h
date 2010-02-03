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

#include "cevec3fwd.h"
#include "cequatfwd.h"

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

extern ce_vec3* ce_camera_get_eye(ce_camera* cam, ce_vec3* eye);
extern ce_vec3* ce_camera_get_forward(ce_camera* cam, ce_vec3* forward);
extern ce_vec3* ce_camera_get_up(ce_camera* cam, ce_vec3* up);
extern ce_vec3* ce_camera_get_right(ce_camera* cam, ce_vec3* right);

extern void ce_camera_set_fov(ce_camera* cam, float fov);
extern void ce_camera_set_aspect(ce_camera* cam, int width, int height);
extern void ce_camera_set_near(ce_camera* cam, float near);
extern void ce_camera_set_far(ce_camera* cam, float far);

extern void ce_camera_set_eye(ce_camera* cam, const ce_vec3* eye);
extern void ce_camera_set_look(ce_camera* cam, const ce_quat* look);

extern void ce_camera_move(ce_camera* cam, float offset_x, float offset_z);
extern void ce_camera_zoom(ce_camera* cam, float offset);
extern void ce_camera_yaw_pitch(ce_camera* cam, float psi, float theta);

extern void ce_camera_setup(ce_camera* cam);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CAMERA_H */
