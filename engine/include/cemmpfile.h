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

#ifndef CE_MMPFILE_H
#define CE_MMPFILE_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum {
	CE_MMP_SIGNATURE = 0x504d4d
};

enum {
	CE_MMP_DXT1 = 0x31545844,
	CE_MMP_DXT3 = 0x33545844,
	CE_MMP_PNT3 = 0x33544e50,
	CE_MMP_R5G6B5 = 0x5650,
	CE_MMP_A1RGB5 = 0x5551,
	CE_MMP_ARGB4 = 0x4444,
	CE_MMP_ARGB8 = 0x8888
};

extern void dxt_compress(unsigned char *dst, unsigned char *src, int format,
						unsigned int width, unsigned int height, int dither);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPFILE_H */
