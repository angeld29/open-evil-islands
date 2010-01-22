#ifndef CE_RESFILE_H
#define CE_RESFILE_H

#include <stdbool.h>
#include <time.h>

#include "memfilefwd.h"
#include "resfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/// Resfile takes ownership of the memfile if successfull.
extern resfile* resfile_open_memfile(const char* name, memfile* mem);
extern resfile* resfile_open_file(const char* path);
extern void resfile_close(resfile* res);

extern const char* resfile_name(const resfile* res);
extern int resfile_node_count(const resfile* res);
extern int resfile_node_index(const char* name, const resfile* res);

extern const char* resfile_node_name(int index, const resfile* res);
extern size_t resfile_node_size(int index, const resfile* res);
extern time_t resfile_node_modified(int index, const resfile* res);
extern bool resfile_node_data(int index, void* data, resfile* res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RESFILE_H */
