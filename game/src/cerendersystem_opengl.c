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

#include <GL/gl.h>

#include "celib.h"
#include "cemath.h"
#include "cealloc.h"
#include "cerendersystem.h"

struct ce_rendersystem {
	int stub;
};

ce_rendersystem* ce_rendersystem_new(void)
{
	ce_rendersystem* rendersystem = ce_alloc(sizeof(ce_rendersystem));
	return rendersystem;
}

void ce_rendersystem_del(ce_rendersystem* rendersystem)
{
	if (NULL != rendersystem) {
		ce_free(rendersystem, sizeof(ce_rendersystem));
	}
}

void ce_rendersystem_apply_transformation(ce_rendersystem* rendersystem,
											const ce_vec3* translation,
											const ce_quat* rotation)
{
	ce_unused(rendersystem);

	glPushMatrix();

	glTranslatef(translation->x,
				translation->y,
				translation->z);

	ce_vec3 rotation_axis;
	float angle = ce_quat_to_angle_axis(rotation, &rotation_axis);

	glRotatef(ce_rad2deg(angle),
				rotation_axis.x,
				rotation_axis.y,
				rotation_axis.z);
}

void ce_rendersystem_discard_transformation(ce_rendersystem* rendersystem)
{
	ce_unused(rendersystem);

	glPopMatrix();
}
