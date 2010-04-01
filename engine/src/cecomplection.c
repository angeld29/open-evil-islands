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

#include "cemath.h"
#include "cecomplection.h"

ce_complection*
ce_complection_init(ce_complection* complection,
					float strength, float dexterity, float height)
{
	complection->strength = strength;
	complection->dexterity = dexterity;
	complection->height = height;
	return complection;
}

ce_complection*
ce_complection_init_array(ce_complection* complection, const float* array)
{
	complection->strength = *array++;
	complection->dexterity = *array++;
	complection->height = *array;
	return complection;
}

ce_complection*
ce_complection_copy(ce_complection* complection, const ce_complection* other)
{
	complection->strength = other->strength;
	complection->dexterity = other->dexterity;
	complection->height = other->height;
	return complection;
}

bool ce_complection_equal(const ce_complection* complection,
							const ce_complection* other)
{
	return ce_fisequal(complection->strength, other->strength, CE_EPS_E3) &&
		ce_fisequal(complection->dexterity, other->dexterity, CE_EPS_E3) &&
		ce_fisequal(complection->height, other->height, CE_EPS_E3);
}
