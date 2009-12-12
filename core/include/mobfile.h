#ifndef CE_MOBFILE_H
#define CE_MOBFILE_H

#include "io.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct mobfile mobfile;

extern mobfile* mobfile_open_callbacks(io_callbacks callbacks,
										void* client_data, const char* name);
extern mobfile* mobfile_open_file(const char* path);

extern int mobfile_close(mobfile* mob);

extern void mobfile_debug_print(mobfile* mob);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MOBFILE_H */
