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

#ifndef CE_OPTPARSE_H
#define CE_OPTPARSE_H

#include <stdbool.h>

#include "cevector.h"
#include "cestring.h"
#include "ceobject.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
    int version_major;
    int version_minor;
    int version_patch;
    ce_string* title;
    ce_string* brief;
    ce_vector* argobjects;
    ce_vector* ctrlobjects;
} ce_optparse;

extern ce_optparse* ce_optparse_new(void);
extern void ce_optparse_del(ce_optparse* optparse);

extern void ce_optparse_set_standard_properties(ce_optparse* optparse,
    int version_major, int version_minor, int version_patch,
    const char* title, const char* brief);

extern bool ce_optparse_get(ce_optparse* optparse, const char* name, void* value);

extern void ce_optparse_add(ce_optparse* optparse, const char* name, ce_type type,
                            const void* value, bool required, const char* shortopt,
                            const char* longopt, const char* glossary);

extern void ce_optparse_add_control(ce_optparse* optparse,
                                    const char* name, const char* glossary);

extern bool ce_optparse_parse(ce_optparse* optparse, int argc, char* argv[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_OPTPARSE_H */
