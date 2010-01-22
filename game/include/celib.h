#ifndef CE_LIB_H
#define CE_LIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int min(int a, int b);
extern int max(int a, int b);
extern size_t smin(size_t a, size_t b);
extern size_t smax(size_t a, size_t b);

extern void swap(int* a, int* b);

extern int clamp(int v, int a, int b);
extern size_t sclamp(size_t v, size_t a, size_t b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LIB_H */
