#ifndef CE_MEMFILE_H
#define CE_MEMFILE_H

#include "ceio.h"
#include "memfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *  You may to instruct memfile to either automatically close or not to close
 *  the resource in memfile_close. Automatic closure is disabled by passing
 *  NULL as the close callback. The client is responsible for closing
 *  a resource when a call to memfile_open_callbacks is unsuccessful.
*/
extern memfile* memfile_open_callbacks(io_callbacks callbacks, void* client_data);
extern memfile* memfile_open_data(void* data, size_t size, const char* mode);
extern memfile* memfile_open_path(const char* path, const char* mode);
extern void memfile_close(memfile* mem);

extern size_t memfile_read(void* data, size_t size, size_t n, memfile* mem);
extern size_t memfile_write(const void* data, size_t size, size_t n, memfile* mem);

extern int memfile_seek(long int offset, int whence, memfile* mem);
extern long int memfile_tell(memfile* mem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MEMFILE_H */
