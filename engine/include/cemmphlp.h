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

#ifndef CE_MMPHLP_H
#define CE_MMPHLP_H

#include "cemmpfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// TODO: refactoring...

extern void ce_mmphlp_decompress_pnt3(uint8_t* restrict dst,
									const uint32_t* restrict src, int size);

extern void ce_mmphlp_a1rgb5_convert_rgb5a1(void* restrict dst,
											const void* restrict src);

extern void ce_mmphlp_argb4_convert_rgba4(void* restrict dst,
											const void* restrict src);

extern void ce_mmphlp_r5g6b5_convert_rgba8(void* restrict dst,
											const void* restrict src);

extern void ce_mmphlp_a1rgb5_convert_rgba8(void* restrict dst,
											const void* restrict src);

extern void ce_mmphlp_argb4_convert_rgba8(void* restrict dst,
											const void* restrict src);

extern void ce_mmphlp_argb8_convert_rgba8(void* restrict dst,
											const void* restrict src);

// TODO: refactoring...

extern void ce_mmphlp_pnt3_convert_argb8(ce_mmpfile* mmpfile);

extern void ce_mmphlp_a1rgb5_swap_rgb5a1(ce_mmpfile* mmpfile);
extern void ce_mmphlp_argb4_swap_rgba4(ce_mmpfile* mmpfile);
extern void ce_mmphlp_argb8_swap_rgba8(ce_mmpfile* mmpfile);

extern void ce_mmphlp_r5g6b5_unpack_rgba8(ce_mmpfile* mmpfile);
extern void ce_mmphlp_a1rgb5_unpack_rgba8(ce_mmpfile* mmpfile);
extern void ce_mmphlp_argb4_unpack_rgba8(ce_mmpfile* mmpfile);
extern void ce_mmphlp_argb8_unpack_rgba8(ce_mmpfile* mmpfile);

extern int ce_mmphlp_storage_requirements_rgba8(int width, int height,
												int mipmap_count);

extern int ce_mmphlp_storage_requirements_dxt(int width, int height,
											int mipmap_count, int format);

extern void ce_mmphlp_rgba8_compress_dxt(ce_mmpfile* mmpfile, int format);
extern void ce_mmphlp_dxt_decompress_rgba8(ce_mmpfile* mmpfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPHLP_H */
