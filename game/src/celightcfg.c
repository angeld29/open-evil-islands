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

#include <stdio.h>

#include "celightcfg.h"

/*
[sunlight]
time00 = 63, 93, 157
time01 = 63, 93, 157
time02 = 92, 113, 165
time03 = 171, 131, 119
time04 = 255, 140, 104
time05 = 255, 198, 129
time06 = 255, 248, 207
time07 = 255, 255, 255
time08 = 255, 255, 255
time09 = 255, 255, 255
time10 = 255, 255, 255
time11 = 255, 255, 255
time12 = 255, 255, 255
time13 = 255, 255, 255
time14 = 255, 255, 255
time15 = 255, 255, 255
time16 = 255, 255, 255
time17 = 255, 255, 255
time18 = 255, 248, 207
time19 = 255, 202, 136
time20 = 255, 123, 82
time21 = 166, 128, 116
time22 = 92, 122, 165
time23 = 63, 93, 157
*/

void ce_lightcfg_read(ce_lightcfg* cfg, const char* path)
{
	FILE* file = fopen(path, "rt");
	if (NULL == file) {
		return;
	}

	char section[16];
	fscanf(file, "%15s", section);

	printf("%s\n", section);

	char data[32];
	fscanf(file, "%31s", data);

	printf("%s\n", data);

	fclose(file);
}
