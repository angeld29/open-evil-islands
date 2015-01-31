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

#ifndef CE_FIGNODE_HPP
#define CE_FIGNODE_HPP

#include "string.hpp"
#include "vector.hpp"
#include "resfile.hpp"
#include "lnkfile.hpp"
#include "figfile.hpp"
#include "bonfile.hpp"
#include "anmfile.hpp"
#include "material.hpp"
#include "renderqueue.hpp"

typedef struct {
    ce_string* name;
    int index;
    ce_figfile* figfile;
    ce_bonfile* bonfile;
    ce_vector* anmfiles;
    ce_material* material;
    ce_rendergroup* rendergroup;
    ce_vector* childs;
} ce_fignode;

extern ce_fignode* ce_fignode_new(ce_res_file* mod_res_file, ce_res_file* bon_res_file, ce_res_file* anm_res_files[], ce_lnkfile* lnkfile);
extern void ce_fignode_del(ce_fignode* fignode);

extern void ce_fignode_accept_renderqueue_cascade(ce_fignode* fignode, ce_renderqueue* renderqueue);

#endif /* CE_FIGNODE_HPP */
