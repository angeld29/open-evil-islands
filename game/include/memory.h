#ifndef CE_MEMORY_H
#define CE_MEMORY_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern bool memory_open(void);
extern void memory_close(void);

extern void* memory_alloc(size_t size);
extern void memory_free(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MEMORY_H */
