#ifndef CE_MPRFILE_H
#define CE_MPRFILE_H

#include "frustum.h"
#include "resfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct mprfile mprfile;

extern mprfile* mprfile_open(resfile* mpr_res, resfile* textures_res);
extern int mprfile_close(mprfile* mpr);

extern float mprfile_get_max_height(const mprfile* mpr);

extern void mprfile_apply_frustum(const frustum* f, mprfile* mpr);
extern void mprfile_render(mprfile* mpr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MPRFILE_H */
