/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

/*
 *  FFmpeg libavcodec initialization.
 *
 *  See also:
 *  1. FFmpeg (C) Michael Niedermayer.
*/

#ifndef CE_AVCODEC_H
#define CE_AVCODEC_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ce_avcodec_init(void);
extern void ce_avcodec_term(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_AVCODEC_H */
