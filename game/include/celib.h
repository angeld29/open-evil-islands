#ifndef CE_LIB_H
#define CE_LIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int cemin(int a, int b);
extern int cemax(int a, int b);
extern size_t cesmin(size_t a, size_t b);
extern size_t cesmax(size_t a, size_t b);

extern void ceswap(int* a, int* b);

extern int ceclamp(int v, int a, int b);
extern size_t cesclamp(size_t v, size_t a, size_t b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LIB_H */
