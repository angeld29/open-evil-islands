#ifndef CE_GLLIB_H
#define CE_GLLIB_H

#include <stdbool.h>

#include <GL/gl.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum {
	GL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	GL_FEATURE_TEXTURE_RECTANGLE,
	GL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	GL_FEATURE_COUNT
};

extern bool gl_query_feature(int feature);

extern GLint gl_max_texture_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GLLIB_H */
