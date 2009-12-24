#ifndef CE_CEMATH_H
#define CE_CEMATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern bool fisequal(float a, float b, float tolerance);

static inline bool fiszero(float a, float tolerance)
{
	return fisequal(a, 0.0f, tolerance);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CEMATH_H */
