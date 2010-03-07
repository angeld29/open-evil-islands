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

#include "cemprhlp.h"

ce_mprfile_material* ce_mprhlp_find_material(ce_mprfile* mprfile,
											ce_mprfile_material_type type)
{
	for (int i = 0, n = mprfile->material_count; i < n; ++i) {
		ce_mprfile_material* material = mprfile->materials + i;
		if (type == material->type) {
			return material;
		}
	}
	return NULL;
}

float* ce_mprhlp_normal2vector(float* vector, uint32_t normal)
{
	vector[0] = (((normal >> 11) & 0x7ff) - 1000.0f) / 1000.0f;
	vector[1] = (normal >> 22) / 1000.0f;
	vector[2] = ((normal & 0x7ff) - 1000.0f) / 1000.0f;
	return vector;
}

int ce_mprhlp_texture_index(uint16_t texture)
{
	return texture & 0x003f;
}

int ce_mprhlp_texture_number(uint16_t texture)
{
	return (texture & 0x3fc0) >> 6;
}

int ce_mprhlp_texture_angle(uint16_t texture)
{
	return (texture & 0xc000) >> 14;
}
