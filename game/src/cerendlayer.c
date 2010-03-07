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

#include "celogging.h"
#include "cealloc.h"
#include "cerendlayer.h"

ce_rendlayer* ce_rendlayer_new(int renditem_count)
{
	ce_rendlayer* rendlayer = ce_alloc_zero(sizeof(ce_rendlayer));
	if (NULL == rendlayer) {
		ce_logging_error("rendlayer: could not allocate memory");
		return NULL;
	}

	if (NULL == (rendlayer->renditems = ce_vector_new_reserved(renditem_count))) {
		ce_rendlayer_del(rendlayer);
		return NULL;
	}

	return rendlayer;
}

void ce_rendlayer_del(ce_rendlayer* rendlayer)
{
	if (NULL != rendlayer) {
		if (NULL != rendlayer->renditems) {
			for (int i = 0, n = ce_vector_count(rendlayer->renditems); i < n; ++i) {
				ce_renditem_del(ce_vector_at(rendlayer->renditems, i));
			}
			ce_vector_del(rendlayer->renditems);
		}
		ce_free(rendlayer, sizeof(ce_rendlayer));
	}
}

void ce_rendlayer_add_renditem(ce_rendlayer* rendlayer, ce_renditem* renditem)
{
	ce_vector_push_back(rendlayer->renditems, renditem);
}
