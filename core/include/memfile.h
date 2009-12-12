#ifndef CE_MEMFILE_H
#define CE_MEMFILE_H

#include <stddef.h>

#include "memfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern memfile* memfile_open(void* data, size_t size, const char* mode);
extern int memfile_close(memfile* mem);

extern size_t memfile_read(void* data, size_t size, size_t n, memfile* mem);
extern size_t memfile_write(const void* data, size_t size,
									size_t n, memfile* mem);

extern int memfile_seek(long int offset, int whence, memfile* mem);
extern long int memfile_tell(memfile* mem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MEMFILE_H */
