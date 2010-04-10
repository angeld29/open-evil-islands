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

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void ce_mmphlp_a1rgb5_convert_rgb5a1(uint16_t* dst, const uint16_t* src,
									int width, int height, int mipmap_count);

extern void ce_mmphlp_argb4_convert_rgba4(uint16_t* dst, const uint16_t* src,
								int width, int height, int mipmap_count);

extern void ce_mmphlp_argb8_convert_rgba8(uint32_t* dst, const uint32_t* src,
								int width, int height, int mipmap_count);

extern void ce_mmphlp_r5g6b5_convert_r8g8b8a8(uint8_t* restrict dst,
	const uint16_t* restrict src, int width, int height, int mipmap_count);

extern void ce_mmphlp_a1rgb5_convert_r8g8b8a8(uint8_t* restrict dst,
	const uint16_t* restrict src, int width, int height, int mipmap_count);

extern void ce_mmphlp_argb4_convert_r8g8b8a8(uint8_t* restrict dst,
	const uint16_t* restrict src, int width, int height, int mipmap_count);

extern void ce_mmphlp_argb8_convert_r8g8b8a8(uint8_t* dst,
	const uint32_t* src, int width, int height, int mipmap_count);

extern void ce_mmphlp_dxt1_convert_r8g8b8a8(uint8_t* restrict dst,
	const uint8_t* restrict src, int width, int height, int mipmap_count);

extern void ce_mmphlp_dxt3_convert_r8g8b8a8(uint8_t* restrict dst,
	const uint8_t* restrict src, int width, int height, int mipmap_count);

extern void ce_mmphlp_r8g8b8a8_convert_dxt1(uint8_t* restrict dst,
	const uint8_t* restrict src, int width, int height, int mipmap_count);

extern void ce_mmphlp_r8g8b8a8_convert_dxt3(uint8_t* restrict dst,
	const uint8_t* restrict src, int width, int height, int mipmap_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPHLP_H */
