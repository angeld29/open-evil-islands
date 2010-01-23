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

#include "celib.h"

#define CE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define CE_CLAMP(v, a, b) ((v) < (a) ? (a) : ((v) > (b) ? (b) : (v)))

int cemin(int a, int b)
{
	return CE_MIN(a, b);
}

int cemax(int a, int b)
{
	return CE_MAX(a, b);
}

size_t cesmin(size_t a, size_t b)
{
	return CE_MIN(a, b);
}

size_t cesmax(size_t a, size_t b)
{
	return CE_MAX(a, b);
}

void ceswap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

int ceclamp(int v, int a, int b)
{
	return CE_CLAMP(v, a, b);
}

size_t cesclamp(size_t v, size_t a, size_t b)
{
	return CE_CLAMP(v, a, b);
}
