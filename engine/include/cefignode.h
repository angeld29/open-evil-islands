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

#ifndef CE_FIGNODE_H
#define CE_FIGNODE_H

#include "cestring.h"
#include "cevector.h"
#include "ceresfile.h"
#include "celnkfile.h"
#include "cefigfile.h"
#include "cebonfile.h"
#include "ceanmfile.h"
#include "cematerial.h"
#include "cerenderqueue.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_string* name;
	int index;
	ce_figfile* figfile;
	ce_bonfile* bonfile;
	ce_vector* anmfiles;
	ce_material* material;
	ce_vector* childs;
} ce_fignode;

extern ce_fignode*
ce_fignode_new(ce_resfile* mod_resfile, ce_resfile* bon_resfile,
				ce_vector* anm_resfiles, ce_lnkfile* lnkfile);
extern void ce_fignode_del(ce_fignode* fignode);

extern void ce_fignode_create_rendergroup_cascade(ce_fignode* fignode,
												ce_renderqueue* renderqueue);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGNODE_H */
