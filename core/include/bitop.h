#ifndef CE_BITOP_H
#define CE_BITOP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

static inline uint8_t bitset(uint8_t val, int pos)
{
	return val | (1 << pos);
}

static inline uint8_t bitclr(uint8_t val, int pos)
{
	return val & ~(1 << pos);
}

static inline uint8_t bitflp(uint8_t val, int pos)
{
	return val ^ (1 << pos);
}

static inline bool bittst(uint8_t val, int pos)
{
	return val & (1 << pos);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BITOP_H */
