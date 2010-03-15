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

#include <stdbool.h>

#include "cevec3.h"
#include "cequat.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	float fov;
	float aspect;
	float near;
	float far;
	ce_vec3 eye;
	ce_quat look;
	bool proj_changed;
	bool eye_changed;
	bool look_changed;
} ce_camera;

extern ce_camera* ce_camera_new(void);
extern void ce_camera_del(ce_camera* camera);

extern ce_vec3* ce_camera_get_forward(ce_camera* camera, ce_vec3* forward);
extern ce_vec3* ce_camera_get_up(ce_camera* camera, ce_vec3* up);
extern ce_vec3* ce_camera_get_right(ce_camera* camera, ce_vec3* right);

extern void ce_camera_set_fov(ce_camera* camera, float fov);
extern void ce_camera_set_aspect(ce_camera* camera, int width, int height);
extern void ce_camera_set_near(ce_camera* camera, float near);
extern void ce_camera_set_far(ce_camera* camera, float far);

extern void ce_camera_set_eye(ce_camera* camera, const ce_vec3* eye);
extern void ce_camera_set_look(ce_camera* camera, const ce_quat* look);

extern void ce_camera_move(ce_camera* camera, float xoffset, float zoffset);
extern void ce_camera_zoom(ce_camera* camera, float offset);
extern void ce_camera_yaw_pitch(ce_camera* camera, float psi, float theta);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CAMERA_H */
