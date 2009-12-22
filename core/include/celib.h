#ifndef CE_CELIB_H
#define CE_CELIB_H

#include <stddef.h>

#define CE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CE_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

static inline int min(int a, int b)
{
	return CE_MIN(a, b);
}

static inline int max(int a, int b)
{
	return CE_MAX(a, b);
}

static inline size_t smin(size_t a, size_t b)
{
	return CE_MIN(a, b);
}

static inline size_t smax(size_t a, size_t b)
{
	return CE_MAX(a, b);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CELIB_H */
