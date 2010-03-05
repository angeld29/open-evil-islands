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

#include "cematerial.h"

ce_material* ce_material_init(ce_material* material)
{
	ce_color_init(&material->ambient, 0.2f, 0.2f, 0.2f, 1.0f);
	ce_color_init(&material->diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
	ce_color_init(&material->specular, 0.0f, 0.0f, 0.0f, 1.0f);
	material->shininess = 0.0f;
	ce_color_init(&material->emission, 0.0f, 0.0f, 0.0f, 1.0f);
	return material;
}
