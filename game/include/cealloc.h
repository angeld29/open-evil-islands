#ifndef CE_ALLOC_H
#define CE_ALLOC_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern bool cealloc_open(void);
extern void cealloc_close(void);

extern void* cealloc(size_t size);
extern void* cealloczero(size_t size);
extern void cefree(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_ALLOC_H */
