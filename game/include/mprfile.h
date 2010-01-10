#ifndef CE_MPRFILE_H
#define CE_MPRFILE_H

#include "resfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct mprfile mprfile;

extern mprfile* mprfile_open(resfile* mpr_res, resfile* textures_res);
extern int mprfile_close(mprfile* mpr);

extern void mprfile_debug_print(mprfile* mpr);
extern void mprfile_debug_render(mprfile* mpr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MPRFILE_H */
