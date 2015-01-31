/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "celogging.h"
#include "cegraphiccontext.h"

void ce_graphic_context_visual_info(int id, int db, int sz, int r,
                                    int g, int b, int a, int dp, int st)
{
    ce_logging_write("graphic context: visual %d chosen", id);
    ce_logging_write("graphic context: +------+----+----+----+----+----+----+----+----+");
    ce_logging_write("graphic context: |   id | db | sz |  r |  g |  b |  a | dp | st |");
    ce_logging_write("graphic context: +------+----+----+----+----+----+----+----+----+");
    ce_logging_write("graphic context: | %4d | %2c | %2d | %2d | %2d | %2d | %2d | %2d | %2d |",
        id, "ny"[db], sz, r, g, b, a, dp, st);
    ce_logging_write("graphic context: +------+----+----+----+----+----+----+----+----+");
    ce_logging_write("graphic context: see GLEW visualinfo for more details");
}