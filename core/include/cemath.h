#ifndef CE_CEMATH_H
#define CE_CEMATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern bool fisequalf(float a, float b, float tolerance);

static inline bool fiszerof(float a, float tolerance)
{
	return fisequalf(a, 0.0f, tolerance);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CEMATH_H */
