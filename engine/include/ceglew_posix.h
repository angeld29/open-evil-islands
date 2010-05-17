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

#ifndef CE_GLEW_PLATFORM_H
#define CE_GLEW_PLATFORM_H

#ifndef __GNUC__
#error not implemented
#endif

#include "ceglew.h"

// HACK: for glew 1.5.4
#ifdef uint
#error already defined
#endif

#define uint GLuint
#include <GL/glxew.h>
#undef uint

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern __thread GLXEWContext ce_glxewcontext;
#define glxewGetContext() (&ce_glxewcontext)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GLEW_PLATFORM_H */
