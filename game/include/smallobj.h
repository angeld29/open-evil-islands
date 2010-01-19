#ifndef CE_SMALLOBJ_H
#define CE_SMALLOBJ_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern bool smallobj_open(void);
extern void smallobj_close(void);

extern void* smallobj_alloc(size_t size);
extern void smallobj_free(void* p, size_t size);

extern size_t smallobj_max_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SMALLOBJ_H */
