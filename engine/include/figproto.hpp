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

#ifndef CE_FIGPROTO_HPP
#define CE_FIGPROTO_HPP

#include <stdbool.h>

#include "string.hpp"
#include "atomic.hpp"
#include "resfile.hpp"
#include "adbfile.hpp"
#include "fignode.hpp"
#include "renderqueue.hpp"

typedef struct {
    int ref_count;
    ce_string* name;
    ce_adb_file* adb_file;
    ce_fignode* fignode;
} ce_figproto;

extern ce_figproto* ce_figproto_new(const char* name, ce_res_file* res_file);
extern void ce_figproto_del(ce_figproto* figproto);

static inline ce_figproto* ce_figproto_add_ref(ce_figproto* figproto)
{
    ce_atomic_inc(int, &figproto->ref_count);
    return figproto;
}

extern void ce_figproto_accept_renderqueue(ce_figproto* figproto, ce_renderqueue* renderqueue);

#endif /* CE_FIGPROTO_HPP */
