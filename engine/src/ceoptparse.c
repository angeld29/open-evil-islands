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

#include <assert.h>

#include <argtable2.h>

#include "cealloc.h"
#include "ceoptparse.h"

ce_optparse* ce_optparse_new(void)
{
	ce_optparse* optparse = ce_alloc(sizeof(ce_optparse));
	optparse->objects = ce_vector_new();
	return optparse;
}

void ce_optparse_del(ce_optparse* optparse)
{
	if (NULL != optparse) {
		ce_vector_for_each(optparse->objects, ce_object_del);
		ce_vector_del(optparse->objects);
		ce_free(optparse, sizeof(ce_optparse));
	}
}
