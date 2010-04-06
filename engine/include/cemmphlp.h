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

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void* ce_mmphlp_decompress_pnt3(void* restrict dst,
										const void* restrict src, int size);

extern void* ce_mmphlp_a1rgb5_to_rgb5a1(void* dst, const void* src,
									int width, int height, int mipmap_count);
extern void* ce_mmphlp_argb4_to_rgba4(void* dst, const void* src,
									int width, int height, int mipmap_count);
extern void* ce_mmphlp_argb8_to_rgba8(void* dst, const void* src,
									int width, int height, int mipmap_count);

extern void* ce_mmphlp_r5g6b5_to_rgba8(void* restrict dst,
									const void* restrict src,
									int width, int height, int mipmap_count);
extern void* ce_mmphlp_a1rgb5_to_rgba8(void* restrict dst,
									const void* restrict src,
									int width, int height, int mipmap_count);
extern void* ce_mmphlp_argb4_to_rgba8(void* restrict dst,
									const void* restrict src,
									int width, int height, int mipmap_count);

extern int ce_mmphlp_storage_requirements_dxt(int width, int height, int format);

extern void* ce_mmphlp_compress_dxt(void* restrict dst, const void* restrict src,
									int width, int height, int format);
extern void* ce_mmphlp_decompress_dxt(void* restrict dst, const void* restrict src,
									int width, int height, int format);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPHLP_H */