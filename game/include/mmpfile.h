#ifndef CE_MMPFILE_H
#define CE_MMPFILE_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*  MPP file format:
 *  signature: uint32 little-endian
 *  width: uint32 little-endian
 *  height: uint32 little-endian
 *  size (PNT3) or mipmap count (other): uint32 little-endian
 *  format: uint32 little-endian
 *  rgb_bit_count: uint32 little-endian
 *  a_bit_mask: uint32 little-endian
 *  a_bit_shift: uint32 little-endian
 *  a_bit_count: uint32 little-endian
 *  r_bit_mask: uint32 little-endian
 *  r_bit_shift: uint32 little-endian
 *  r_bit_count: uint32 little-endian
 *  g_bit_mask: uint32 little-endian
 *  g_bit_shift: uint32 little-endian
 *  g_bit_count: uint32 little-endian
 *  b_bit_mask: uint32 little-endian
 *  b_bit_shift: uint32 little-endian
 *  b_bit_count: uint32 little-endian
 *  user_data_shift: uint32 little-endian
*/

enum {
	MMP_SIGNATURE = 0x504d4d
};

enum {
	MMP_DXT1 = 0x31545844,
	MMP_DXT3 = 0x33545844,
	MMP_PNT3 = 0x33544e50,
	MMP_R5G6B5 = 0x5650,
	MMP_A1RGB5 = 0x5551,
	MMP_ARGB4 = 0x4444,
	MMP_ARGB8 = 0x8888
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPFILE_H */
