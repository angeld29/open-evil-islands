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

#include "cealloc.h"
#include "cerenderwindow.h"

void ce_renderwindow_add_listener(ce_renderwindow* renderwindow,
									ce_renderwindow_listener* listener)
{
	ce_vector_push_back(renderwindow->listeners, listener);
}

void ce_renderwindow_emit_closed(ce_renderwindow* renderwindow)
{
	for (int i = 0; i < renderwindow->listeners->count; ++i) {
		ce_renderwindow_listener* listener = renderwindow->listeners->items[i];
		(*listener->closed)(listener->listener);
	}
}
