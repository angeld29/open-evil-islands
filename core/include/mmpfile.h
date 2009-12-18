#ifndef CE_MMPFILE_H
#define CE_MMPFILE_H

#include <GL/gl.h>

#include "memfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern GLuint mmpfile_gentex(memfile* mem);
extern GLuint mmpfile_gentex_file(const char* path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPFILE_H */
