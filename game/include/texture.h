#ifndef CE_MMPFILE_H
#define CE_MMPFILE_H

#include <GL/gl.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *  @param id Generate a new texture id if 0, otherwise reuse the texture id.
 *  @return The OpenGL texture handle if successfull or 0 if failed.
*/
extern GLuint mmpfile_generate_texture(GLuint id, void* data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPFILE_H */
